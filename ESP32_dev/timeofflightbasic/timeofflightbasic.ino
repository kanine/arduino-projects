// timeofflightbasic – ESP32
//
// Polls the VL53L1X ToF sensor at a configurable rate, batches readings,
// and posts them as a JSON array to WEBHOOK_URL at a configurable interval.
// Configuration lives in secrets.h (copy from secrets.h.example).

// ── OTA config ────────────────────────────────────────────────────────────────
#define OTA_LED_PIN  2
#include "ota_core.h"

// ── HTTP config ───────────────────────────────────────────────────────────────
#define HTTP_URL     WEBHOOK_URL   // defined in secrets.h
#define HTTP_LED_PIN 2
#include "http_core.h"

// ── Sensor ────────────────────────────────────────────────────────────────────
#include <Wire.h>
#include <time.h>
#include "Adafruit_VL53L1X.h"

// ── User parameters ───────────────────────────────────────────────────────────
#include "config.h"

// ── Runtime config (initialised from config.h; updated from server responses) ──
static float    pollsPerMinute = POLLS_PER_MINUTE;
static float    windowSeconds  = WINDOW_SECONDS;
static uint32_t pollMs;    // derived: 60000 / pollsPerMinute, clamped to 20 ms floor
static uint32_t batchMs;   // derived: windowSeconds * 1000
static uint16_t batchCap;  // derived: max readings to keep in one batch window

constexpr uint16_t MAX_READINGS = 500;   // hard cap; each reading = 6 bytes

// ── recomputeDerivedTiming ───────────────────────────────────────────────────
// Keeps derived runtime timing values and per-window sample cap in sync with
// current pollsPerMinute/windowSeconds configuration.
static void recomputeDerivedTiming() {
  uint32_t rawPoll = (uint32_t)(60000.0f / pollsPerMinute);
  pollMs = rawPoll < 20 ? 20 : rawPoll;

  batchMs = (uint32_t)(windowSeconds * 1000.0f);
  if (batchMs == 0) batchMs = 1;

  uint32_t cap = batchMs / pollMs;
  if (cap == 0) cap = 1;
  if (cap > MAX_READINGS) cap = MAX_READINGS;
  batchCap = (uint16_t)cap;
}

// ── applyServerConfig ─────────────────────────────────────────────────────────
// Parses the "config" block from a server response and updates runtime timing
// variables if the server has sent new values.
static void applyServerConfig(const String& body) {
  bool changed = false;
  int idx = body.indexOf("\"polls_per_minute\":");
  if (idx >= 0) {
    float v = body.substring(idx + 19).toFloat();
    if (v > 0 && v != pollsPerMinute) {
      pollsPerMinute = v;
      changed = true;
    }
  }
  idx = body.indexOf("\"window_seconds\":");
  if (idx >= 0) {
    float v = body.substring(idx + 17).toFloat();
    if (v > 0 && v != windowSeconds) {
      windowSeconds = v;
      changed = true;
    }
  }

  if (changed) {
    recomputeDerivedTiming();
    Serial.printf("[cfg] updated ppm=%.1f poll_ms=%u window_s=%.1f batch_ms=%u cap=%u\n",
                  pollsPerMinute, pollMs, windowSeconds, batchMs, batchCap);
  }
}

// ── Reading storage ───────────────────────────────────────────────────────────
struct Reading { uint64_t ts_ms; int16_t distance_mm; };
static Reading  readings[MAX_READINGS];
static uint16_t readingCount = 0;
static uint32_t batchId      = 0;

// ── State ─────────────────────────────────────────────────────────────────────
static Adafruit_VL53L1X vl53;
static bool     sensorOk  = false;
static uint32_t lastPoll  = 0;
static uint32_t lastBatch = 0;

// ── Setup ─────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(1500);   // let monitor connect before first output
  pinMode(OTA_LED_PIN, OUTPUT);
  digitalWrite(OTA_LED_PIN, LOW);

  // Initialise derived timing from startup config
  recomputeDerivedTiming();

  otaCoreSetup();

  // Sync time via NTP — required for Unix timestamps in batch payloads
  configTime(0, 0, "pool.ntp.org");
  Serial.print("[time] syncing NTP");
  uint32_t t0 = millis();
  while (time(nullptr) < 1000000000UL && millis() - t0 < 8000) {
    delay(200);
    Serial.print(".");
  }
  Serial.printf(" %s\n", time(nullptr) > 1000000000UL ? "ok" : "TIMEOUT — timestamps will be 0");

  Wire.begin();   // SDA=D21, SCL=D22
  if (!vl53.begin(0x29, &Wire)) {
    Serial.println("[tof] sensor not found — check wiring");
  } else {
    vl53.VL53L1X_SetDistanceMode(1);   // 1 = Short (~1.3 m max)
    vl53.setTimingBudget(20);          // 20 ms minimum for short mode
    vl53.startRanging();
    sensorOk = true;
    Serial.printf("[tof] ready — %.1f polls/min (%u ms), %.1f s window, cap %u (hard max %u)\n",
                  pollsPerMinute, pollMs, windowSeconds, batchCap, MAX_READINGS);
  }
}

// ── sendBatch ─────────────────────────────────────────────────────────────────
static void sendBatch() {
  if (readingCount == 0) {
    Serial.println("[main] batch empty — skipping");
    return;
  }

  float    cpu_temp_c = temperatureRead();
  uint32_t uptime_s   = millis() / 1000;
  char uptime_fmt[20];
  snprintf(uptime_fmt, sizeof(uptime_fmt), "%ud %02u:%02u:%02u",
           uptime_s / 86400,
           (uptime_s % 86400) / 3600,
           (uptime_s % 3600) / 60,
           uptime_s % 60);

  batchId++;
  uint64_t startTs = readings[0].ts_ms;
  uint64_t endTs   = readings[readingCount - 1].ts_ms;

  // Allocate JSON buffer: header ~300 + each reading ~50 + footer ~3
  size_t bufSize = 400 + (size_t)readingCount * 50;
  char*  json    = (char*)malloc(bufSize);
  if (!json) {
    Serial.println("[main] malloc failed — skipping batch");
    readingCount = 0;
    return;
  }

  int pos = snprintf(json, bufSize,
    "{\"app\":\"timeofflightbasic\",\"host\":\"%s\","
    "\"batch_id\":%lu,\"start_time_ms\":%llu,\"end_time_ms\":%llu,"
    "\"uptime\":\"%s\",\"cpu_temp_c\":%.1f,"
    "\"config\":{\"polls_per_minute\":%.1f,\"window_seconds\":%.1f,\"poll_ms\":%lu,\"batch_cap\":%u},"
    "\"readings\":[",
    OTA_HOSTNAME, batchId, startTs, endTs,
    uptime_fmt, cpu_temp_c,
    pollsPerMinute, windowSeconds, pollMs, batchCap);

  for (uint16_t i = 0; i < readingCount && pos < (int)bufSize - 50; i++) {
    pos += snprintf(json + pos, bufSize - pos,
      "%s{\"ts_ms\":%llu,\"distance_mm\":%d}",
      i == 0 ? "" : ",",
      readings[i].ts_ms, readings[i].distance_mm);
  }
  snprintf(json + pos, bufSize - pos, "]}");

  Serial.printf("[main] batch #%lu — %u readings  %llu → %llu\n",
                batchId, readingCount, startTs, endTs);
  bool ok = httpPost(json);
  Serial.printf("[main] result : %s\n\n", ok ? "success" : "FAILED — check webhook");
  if (ok) {
    applyServerConfig(httpLastBody);
    Serial.printf("[cfg] effective ppm=%.1f poll_ms=%u window_s=%.1f batch_ms=%u cap=%u\n",
                  pollsPerMinute, pollMs, windowSeconds, batchMs, batchCap);
  }

  free(json);
  readingCount = 0;
}

// ── Loop ──────────────────────────────────────────────────────────────────────
void loop() {
  otaCoreHandle();
  if (!otaCoreReady()) return;

  uint32_t now = millis();

  // ── Poll sensor ───────────────────────────────────────────────────────────
  if (sensorOk && now - lastPoll >= pollMs) {
    lastPoll = now;
    if (vl53.dataReady()) {
      uint32_t d = 0;
      vl53.GetDistance(&d);
      if (d > 0 && readingCount < batchCap) {
        readings[readingCount++] = { (uint64_t)time(nullptr) * 1000ULL + (millis() % 1000), (int16_t)d };
      }
      vl53.clearInterrupt();
    }
  }

  // ── Send batch on interval ────────────────────────────────────────────────
  if (now - lastBatch >= batchMs) {
    sendBatch();
    // Anchor next window after send finishes so config updates (e.g. 5 s)
    // take effect as observed wall-clock spacing in serial logs.
    lastBatch = millis();
  }
}
