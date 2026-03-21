// httpcore – ESP32 reference sketch for http_core.h
//
// Posts a JSON heartbeat to WEBHOOK_URL every INTERVAL_MS.
// Use this as a minimal integration test for http_core.h.
// Configuration lives in secrets.h (copy from secrets.h.example).

// ── OTA config ────────────────────────────────────────────────────────────────
#define OTA_LED_PIN  2
#include "ota_core.h"

// ── HTTP config ───────────────────────────────────────────────────────────────
#define HTTP_URL     WEBHOOK_URL   // defined in secrets.h
#define HTTP_LED_PIN 2
#include "http_core.h"

// ── User parameters ───────────────────────────────────────────────────────────
constexpr uint32_t INTERVAL_MS = 10000;   // ms between posts

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
    char json[64];
    snprintf(json, sizeof(json), "{\"source\":\"httpcore\",\"uptime\":%lu}", now);
    httpPost(json);
  }
}
