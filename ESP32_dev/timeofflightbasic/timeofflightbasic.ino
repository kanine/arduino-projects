// timeofflightbasic – ESP32
//
// Reads distance from an Adafruit VL53L1X ToF sensor (short mode, 50 ms
// timing budget) and posts JSON readings to WEBHOOK_URL every INTERVAL_MS.
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
#include "Adafruit_VL53L1X.h"

// ── User parameters ───────────────────────────────────────────────────────────
constexpr uint32_t INTERVAL_MS   = 5000;   // ms between webhook posts
constexpr uint16_t TIMING_BUDGET = 50;     // ms; short mode minimum is 20 ms

// ── State ─────────────────────────────────────────────────────────────────────
static Adafruit_VL53L1X vl53;
static bool     sensorOk        = false;
static int16_t  lastDistance_mm = -1;
static uint32_t lastPost        = 0;

// ── Setup ─────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(1500);   // let monitor connect before first output
  pinMode(OTA_LED_PIN, OUTPUT);
  digitalWrite(OTA_LED_PIN, LOW);

  Wire.begin();                          // SDA=GPIO21, SCL=GPIO22
  if (!vl53.begin(0x29, &Wire)) {
    Serial.println("[tof] sensor not found — check wiring");
  } else {
    vl53.VL53L1X_SetDistanceMode(1);     // 1 = Short (~1.3 m max)
    vl53.setTimingBudget(TIMING_BUDGET);
    vl53.startRanging();
    sensorOk = true;
    Serial.println("[tof] sensor ready — short mode, 50 ms budget");
  }

  otaCoreSetup();
}

// ── Loop ──────────────────────────────────────────────────────────────────────
void loop() {
  otaCoreHandle();
  if (!otaCoreReady()) return;

  // ── Poll sensor ───────────────────────────────────────────────────────────
  if (sensorOk) {
    if (vl53.dataReady()) {
      uint32_t d = 0;
      vl53.GetDistance(&d);
      if (d > 0) {
        lastDistance_mm = (int16_t)d;
      }
      vl53.clearInterrupt();
    }
  }

  // ── Post on interval ──────────────────────────────────────────────────────
  uint32_t now = millis();
  if (now - lastPost >= INTERVAL_MS) {
    lastPost = now;

    float    cpu_temp_c = temperatureRead();
    uint32_t uptime_s   = now / 1000;
    uint32_t days  = uptime_s / 86400;
    uint32_t hours = (uptime_s % 86400) / 3600;
    uint32_t mins  = (uptime_s % 3600) / 60;
    uint32_t secs  = uptime_s % 60;
    char uptime_fmt[20];
    snprintf(uptime_fmt, sizeof(uptime_fmt), "%ud %02u:%02u:%02u", days, hours, mins, secs);

    char json[240];
    if (lastDistance_mm >= 0) {
      snprintf(json, sizeof(json),
        "{\"app\":\"timeofflightbasic\",\"host\":\"%s\",\"uptime_ms\":%lu,\"uptime\":\"%s\",\"cpu_temp_c\":%.1f,\"distance_mm\":%d}",
        OTA_HOSTNAME, now, uptime_fmt, cpu_temp_c, lastDistance_mm);
    } else {
      snprintf(json, sizeof(json),
        "{\"app\":\"timeofflightbasic\",\"host\":\"%s\",\"uptime_ms\":%lu,\"uptime\":\"%s\",\"cpu_temp_c\":%.1f,\"distance_mm\":null}",
        OTA_HOSTNAME, now, uptime_fmt, cpu_temp_c);
    }

    Serial.printf("[main] posting: %s\n", json);
    bool ok = httpPost(json);
    Serial.printf("[main] result : %s\n\n", ok ? "success" : "FAILED — check webhook");
  }
}
