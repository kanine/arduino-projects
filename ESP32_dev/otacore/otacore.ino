// otacore – ESP32 OTA demo / template
//
// This sketch demonstrates the minimum integration of ota_core.h.
// Copy ota_core.h and secrets.h into a new sketch folder to reuse it.
//
// Configuration lives in secrets.h (copy from secrets.h.example).

// ── Optional OTA config (must be defined before the include) ──────────────────
#define OTA_LED_PIN  2          // onboard blue LED; remove if no LED wired

#include "ota_core.h"

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

  // Guard: don't run sketch logic until WiFi + OTA are ready.
  if (!otaCoreReady()) return;

  // ── Your sketch logic here ───────────────────────────────────────────────
}
