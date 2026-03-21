// touch-sensor – ESP32
//
// Tests capacitive touch on T0 (GPIO4).
// Onboard LED (GPIO2) lights when the pin is touched, goes out when released.
// Configuration lives in secrets.h (copy from secrets.h.example).

// ── Optional OTA config ────────────────────────────────────────────────────────
#define OTA_LED_PIN  2          // onboard blue LED; OTA status blink + touch feedback

#include "ota_core.h"

// ── User parameters ───────────────────────────────────────────────────────────
constexpr uint8_t  TOUCH_PIN       = T0;   // GPIO4 — attach a short wire as touch pad
constexpr uint16_t TOUCH_THRESHOLD = 700;  // touched when reading drops below this value
constexpr uint32_t SERIAL_INTERVAL = 200;  // ms between serial prints

// ── State ─────────────────────────────────────────────────────────────────────
static bool     lastTouched  = false;
static uint32_t lastPrint    = 0;

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
  uint16_t val     = touchRead(TOUCH_PIN);
  bool     touched = (val < TOUCH_THRESHOLD);

  // LED follows touch state; OTA callbacks will override during flash (acceptable)
  if (touched != lastTouched) {
    lastTouched = touched;
    digitalWrite(OTA_LED_PIN, touched ? HIGH : LOW);
    Serial.printf("[touch] %s  (raw=%u)\n", touched ? "TOUCHED" : "released", val);
  }

  // Periodic raw value log for threshold tuning
  uint32_t now = millis();
  if (now - lastPrint >= SERIAL_INTERVAL) {
    lastPrint = now;
    Serial.printf("[touch] raw=%u  threshold=%u\n", val, TOUCH_THRESHOLD);
  }
}
