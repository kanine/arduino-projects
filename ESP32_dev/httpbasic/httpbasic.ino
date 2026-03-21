// httpbasic – ESP32
//
// Demonstrates http_core.h: posts a JSON heartbeat to WEBHOOK_URL every
// INTERVAL_MS and prints results to serial so both the IDE monitor and the
// webhook can observe the output simultaneously.
// Configuration lives in secrets.h (copy from secrets.h.example).

// ── OTA config ────────────────────────────────────────────────────────────────
#define OTA_LED_PIN  2
#include "ota_core.h"

// ── HTTP config ───────────────────────────────────────────────────────────────
#define HTTP_URL     WEBHOOK_URL   // defined in secrets.h
#define HTTP_LED_PIN 2
#include "http_core.h"

// ── User parameters ───────────────────────────────────────────────────────────
constexpr uint32_t INTERVAL_MS = 10000;   // ms between webhook posts

// ── State ─────────────────────────────────────────────────────────────────────
static uint32_t lastPost = 0;

// ── Setup ─────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  pinMode(OTA_LED_PIN, OUTPUT);
  digitalWrite(OTA_LED_PIN, LOW);

  otaCoreSetup();
}

// ── Loop ──────────────────────────────────────────────────────────────────────
void loop() {
  otaCoreHandle();
  if (!otaCoreReady()) return;

  // ── Sketch logic ──────────────────────────────────────────────────────────
  uint32_t now = millis();
  if (now - lastPost >= INTERVAL_MS) {
    lastPost = now;

    char json[128];
    snprintf(json, sizeof(json),
      "{\"source\":\"httpbasic\",\"uptime_ms\":%lu}", now);

    Serial.printf("[main] posting: %s\n", json);
    bool ok = httpPost(json);
    Serial.printf("[main] result : %s\n\n", ok ? "success" : "FAILED — check webhook");
  }
}
