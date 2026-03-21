// otablink – ESP32
// Connects to WiFi and starts OTA. Blinks the onboard LED three times fast
// once WiFi and OTA are confirmed ready.
//
// Configuration lives in secrets.h (copy from secrets.h.example).

// ── OTA config ────────────────────────────────────────────────────────────────
#define OTA_LED_PIN  2          // onboard blue LED (GPIO2, active HIGH)

#include "ota_core.h"

// ── State ─────────────────────────────────────────────────────────────────────
static unsigned long lastBlinkMs  = 0;
static bool          ledState     = false;

// ── Setup ─────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  pinMode(OTA_LED_PIN, OUTPUT);
  digitalWrite(OTA_LED_PIN, LOW);

  otaCoreSetup();

  if (otaCoreReady()) {
    digitalWrite(OTA_LED_PIN, HIGH);
    delay(3000);
    digitalWrite(OTA_LED_PIN, LOW);
    Serial.println("[otablink] Ready.");
  }
}

// ── Loop ──────────────────────────────────────────────────────────────────────
void loop() {
  otaCoreHandle();
  if (!otaCoreReady()) return;

  // ── 1-second blink ────────────────────────────────────────────────────────
  unsigned long now = millis();
  if (now - lastBlinkMs >= 1000) {
    lastBlinkMs = now;
    ledState = !ledState;
    digitalWrite(OTA_LED_PIN, ledState ? HIGH : LOW);
  }
}
