// OTA basic – ESP32
// Connects to WiFi and starts ArduinoOTA, allowing wireless sketch uploads.
// LED blinks on OTA activity; reconnects automatically if WiFi drops.
//
// Configuration lives in secrets.h (copy from secrets.h.example).
// Required libraries (bundled with ESP32 Arduino core):
//   - WiFi
//   - ArduinoOTA

#include <WiFi.h>
#include <ArduinoOTA.h>
#include "secrets.h"

// ── User parameters ───────────────────────────────────────────────────────────
static const uint8_t LED_PIN = 2;   // onboard blue LED (GPIO2, active HIGH)

// ── Constants ─────────────────────────────────────────────────────────────────
static const unsigned long WIFI_TIMEOUT_MS   = 15000;
static const unsigned long RECONNECT_WAIT_MS =  5000;

// ── State ─────────────────────────────────────────────────────────────────────
static unsigned long lastReconnect = 0;
static bool          otaReady      = false;

// ── Helpers ───────────────────────────────────────────────────────────────────
static void blinkLed(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(120);
    digitalWrite(LED_PIN, LOW);
    if (i < times - 1) delay(120);
  }
}

static bool connectWiFi() {
  Serial.printf("Connecting to SSID: %s\n", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - start >= WIFI_TIMEOUT_MS) {
      Serial.println("WiFi connect timed out.");
      return false;
    }
    delay(250);
    Serial.print(".");
  }
  Serial.printf("\nConnected. IP: %s\n", WiFi.localIP().toString().c_str());
  return true;
}

static void setupOTA() {
  ArduinoOTA.setHostname(OTA_HOSTNAME);

#ifdef OTA_PASSWORD
  ArduinoOTA.setPassword(OTA_PASSWORD);
#endif

  ArduinoOTA.onStart([]() {
    String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
    Serial.printf("OTA start: updating %s\n", type.c_str());
    digitalWrite(LED_PIN, HIGH);
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("\nOTA complete.");
    digitalWrite(LED_PIN, LOW);
    blinkLed(3);
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("OTA progress: %u%%\r", progress * 100 / total);
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("\nOTA error[%u]: ", error);
    if      (error == OTA_AUTH_ERROR)    Serial.println("auth failed");
    else if (error == OTA_BEGIN_ERROR)   Serial.println("begin failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("connect failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("receive failed");
    else if (error == OTA_END_ERROR)     Serial.println("end failed");
    blinkLed(5);
  });

  ArduinoOTA.begin();
  Serial.printf("OTA ready. Hostname: %s\n", OTA_HOSTNAME);
  Serial.printf("           IP      : %s\n", WiFi.localIP().toString().c_str());
  otaReady = true;
  blinkLed(2);
}

// ── Setup ─────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  delay(500);

  Serial.println("\n── OTA Basic ────────────────────────────────────────");
  Serial.printf("Hostname: %s\n", OTA_HOSTNAME);

  if (connectWiFi()) {
    setupOTA();
  }
}

// ── Loop ──────────────────────────────────────────────────────────────────────
void loop() {
  unsigned long now = millis();

  // ── Reconnect if dropped ────────────────────────────────────────────────
  if (WiFi.status() != WL_CONNECTED) {
    otaReady = false;
    if (now - lastReconnect >= RECONNECT_WAIT_MS) {
      lastReconnect = now;
      Serial.println("WiFi lost. Reconnecting...");
      WiFi.disconnect();
      if (connectWiFi()) {
        setupOTA();
      }
    }
    return;
  }

  // ── Handle OTA ──────────────────────────────────────────────────────────
  if (otaReady) {
    ArduinoOTA.handle();
  }
}
