// ota_core.h — header-only OTA + WiFi module for ESP32
//
// Drop this file into any ESP32 sketch folder alongside secrets.h, then:
//
//   void setup() { otaCoreSetup(); }
//   void loop()  { otaCoreHandle(); }
//
// Optional: define any of the following BEFORE including this header:
//
//   #define OTA_LED_PIN        2      // GPIO for status blink (omit to disable)
//   #define OTA_WIFI_TIMEOUT_MS 15000 // WiFi connect timeout  (default: 15000)
//   #define OTA_RECONNECT_MS    5000  // Reconnect retry interval (default: 5000)
//
// Credentials and hostname come from secrets.h:
//   WIFI_SSID, WIFI_PASSWORD, OTA_HOSTNAME
//   OTA_PASSWORD (optional — comment out to disable auth)

#pragma once

#include <WiFi.h>
#include <ArduinoOTA.h>
#include "secrets.h"

// ── Optional config defaults ───────────────────────────────────────────────────
#ifndef OTA_WIFI_TIMEOUT_MS
#  define OTA_WIFI_TIMEOUT_MS  15000UL
#endif

#ifndef OTA_RECONNECT_MS
#  define OTA_RECONNECT_MS      5000UL
#endif

// ── Internal state ─────────────────────────────────────────────────────────────
static unsigned long _otaLastReconnect = 0;
static bool          _otaReady         = false;

// ── LED helpers (no-ops when OTA_LED_PIN not defined) ─────────────────────────
#ifdef OTA_LED_PIN
static void _otaBlink(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(OTA_LED_PIN, HIGH); delay(120);
    digitalWrite(OTA_LED_PIN, LOW);
    if (i < times - 1) delay(120);
  }
}
static void _otaLedOn()  { digitalWrite(OTA_LED_PIN, HIGH); }
static void _otaLedOff() { digitalWrite(OTA_LED_PIN, LOW);  }
#else
static void _otaBlink(int) {}
static void _otaLedOn()   {}
static void _otaLedOff()  {}
#endif

// ── WiFi connect ───────────────────────────────────────────────────────────────
static bool _otaConnectWiFi() {
  Serial.printf("[ota] Connecting to %s", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - start >= OTA_WIFI_TIMEOUT_MS) {
      Serial.println(" timed out.");
      return false;
    }
    delay(250);
    Serial.print(".");
  }
  Serial.printf(" IP: %s\n", WiFi.localIP().toString().c_str());
  return true;
}

// ── OTA init ───────────────────────────────────────────────────────────────────
static void _otaBegin() {
  ArduinoOTA.setHostname(OTA_HOSTNAME);

#ifdef OTA_PASSWORD
  ArduinoOTA.setPassword(OTA_PASSWORD);
#endif

  ArduinoOTA.onStart([]() {
    String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
    Serial.printf("[ota] Start: %s\n", type.c_str());
    _otaLedOn();
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("[ota] Complete.");
    _otaLedOff();
    _otaBlink(3);
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("[ota] %u%%\r", progress * 100 / total);
  });

  ArduinoOTA.onError([](ota_error_t error) {
    const char* msg = "unknown";
    if      (error == OTA_AUTH_ERROR)    msg = "auth failed";
    else if (error == OTA_BEGIN_ERROR)   msg = "begin failed";
    else if (error == OTA_CONNECT_ERROR) msg = "connect failed";
    else if (error == OTA_RECEIVE_ERROR) msg = "receive failed";
    else if (error == OTA_END_ERROR)     msg = "end failed";
    Serial.printf("[ota] Error: %s\n", msg);
    _otaBlink(5);
  });

  ArduinoOTA.begin();
  Serial.printf("[ota] Ready — hostname: %s  IP: %s\n",
    OTA_HOSTNAME, WiFi.localIP().toString().c_str());
  _otaReady = true;
  _otaBlink(2);
}

// ── Public API ─────────────────────────────────────────────────────────────────

// Call once in setup(). Connects WiFi and starts OTA.
// If OTA_LED_PIN is defined, call pinMode(OTA_LED_PIN, OUTPUT) before this.
static void otaCoreSetup() {
  if (_otaConnectWiFi()) {
    _otaBegin();
  }
}

// Call every loop(). Handles OTA requests and WiFi reconnection.
static void otaCoreHandle() {
  if (WiFi.status() != WL_CONNECTED) {
    _otaReady = false;
    unsigned long now = millis();
    if (now - _otaLastReconnect >= OTA_RECONNECT_MS) {
      _otaLastReconnect = now;
      Serial.println("[ota] WiFi lost — reconnecting...");
      WiFi.disconnect();
      if (_otaConnectWiFi()) _otaBegin();
    }
    return;
  }
  if (_otaReady) ArduinoOTA.handle();
}

// Returns true when WiFi is up and OTA is running.
// Use this to guard sketch logic that needs a network connection.
static bool otaCoreReady() {
  return _otaReady;
}
