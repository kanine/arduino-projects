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

// ── Derived timing ────────────────────────────────────────────────────────────
constexpr uint32_t _rawPollMs  = (uint32_t)(60000.0f / POLLS_PER_MINUTE);
constexpr uint32_t POLL_MS     = _rawPollMs < 20 ? 20 : _rawPollMs;  // 20 ms sensor floor
constexpr uint32_t BATCH_MS    = (uint32_t)(BATCH_SIZE_MIN * 60000.0f);
constexpr uint16_t MAX_READINGS = 500;   // hard cap; each reading = 6 bytes

// ── Reading storage ───────────────────────────────────────────────────────────
struct Reading { time_t ts; int16_t distance_mm; };
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
    Serial.printf("[tof] ready — %u ms poll, %.1f min batch, max %u readings\n",
                  POLL_MS, BATCH_SIZE_MIN, MAX_READINGS);
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
  time_t startTs = readings[0].ts;
  time_t endTs   = readings[readingCount - 1].ts;

  // Allocate JSON buffer: header ~300 + each reading ~38 + footer ~3
  size_t bufSize = 400 + (size_t)readingCount * 42;
  char*  json    = (char*)malloc(bufSize);
  if (!json) {
    Serial.println("[main] malloc failed — skipping batch");
    readingCount = 0;
    return;
  }

  int pos = snprintf(json, bufSize,
    "{\"app\":\"timeofflightbasic\",\"host\":\"%s\","
    "\"batch_id\":%lu,\"start_time\":%ld,\"end_time\":%ld,"
    "\"uptime\":\"%s\",\"cpu_temp_c\":%.1f,"
    "\"config\":{\"polls_per_min\":%.1f,\"batch_size_min\":%.1f,\"poll_ms\":%lu},"
    "\"readings\":[",
    OTA_HOSTNAME, batchId, (long)startTs, (long)endTs,
    uptime_fmt, cpu_temp_c,
    (float)POLLS_PER_MINUTE, (float)BATCH_SIZE_MIN, POLL_MS);

  for (uint16_t i = 0; i < readingCount && pos < (int)bufSize - 42; i++) {
    pos += snprintf(json + pos, bufSize - pos,
      "%s{\"ts\":%ld,\"distance_mm\":%d}",
      i == 0 ? "" : ",",
      (long)readings[i].ts, readings[i].distance_mm);
  }
  snprintf(json + pos, bufSize - pos, "]}");

  Serial.printf("[main] batch #%lu — %u readings  %ld → %ld\n",
                batchId, readingCount, (long)startTs, (long)endTs);
  bool ok = httpPost(json);
  Serial.printf("[main] result : %s\n\n", ok ? "success" : "FAILED — check webhook");

  free(json);
  readingCount = 0;
}

// ── Loop ──────────────────────────────────────────────────────────────────────
void loop() {
  otaCoreHandle();
  if (!otaCoreReady()) return;

  uint32_t now = millis();

  // ── Poll sensor ───────────────────────────────────────────────────────────
  if (sensorOk && now - lastPoll >= POLL_MS) {
    lastPoll = now;
    if (vl53.dataReady()) {
      uint32_t d = 0;
      vl53.GetDistance(&d);
      if (d > 0 && readingCount < MAX_READINGS) {
        readings[readingCount++] = { time(nullptr), (int16_t)d };
      }
      vl53.clearInterrupt();
    }
  }

  // ── Send batch on interval ────────────────────────────────────────────────
  if (now - lastBatch >= BATCH_MS) {
    lastBatch = now;
    sendBatch();
  }
}
