// http_core.h — header-only HTTP POST module for ESP32
//
// Drop this file into any ESP32 sketch folder, define HTTP_URL, then call
// httpPost() to send JSON payloads to a webhook endpoint.
//
// Required — define BEFORE including this header:
//   #define HTTP_URL "https://your.webhook/endpoint"
//
// Optional defines:
//   #define HTTP_LED_PIN    2      // GPIO to blink 3× on failure
//   #define HTTP_TIMEOUT_MS 5000   // request timeout ms (default: 5000)
//
// Requires WiFi to be already connected (pair with ota_core.h).
//
// API:
//   bool httpPost(const char* json);
//     POSTs json to HTTP_URL with Content-Type: application/json.
//     Returns true when the server responds with {"success": true}.
//     On network error or missing success field: blinks HTTP_LED_PIN 3×,
//     logs to serial, and returns false.

#pragma once

#include <HTTPClient.h>

#ifndef HTTP_TIMEOUT_MS
#  define HTTP_TIMEOUT_MS 5000UL
#endif

// ── LED helpers (no-ops when HTTP_LED_PIN not defined) ─────────────────────────
#ifdef HTTP_LED_PIN
static void _httpBlink(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(HTTP_LED_PIN, HIGH); delay(80);
    digitalWrite(HTTP_LED_PIN, LOW);
    if (i < times - 1) delay(80);
  }
}
#else
static void _httpBlink(int) {}
#endif

// ── httpPost ───────────────────────────────────────────────────────────────────
// Sends json to HTTP_URL via HTTP POST (Content-Type: application/json).
// Expects the server to reply with JSON containing "success": true.
// Returns true on confirmed success, false otherwise.
static bool httpPost(const char* json) {
  HTTPClient http;
  http.begin(HTTP_URL);
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(HTTP_TIMEOUT_MS);

  Serial.printf("[http] POST %s  body: %s\n", HTTP_URL, json);

  int code = http.POST((uint8_t*)json, strlen(json));

  if (code <= 0) {
    Serial.printf("[http] Error: %s\n", HTTPClient::errorToString(code).c_str());
    http.end();
    _httpBlink(3);
    return false;
  }

  String body = http.getString();
  http.end();

  Serial.printf("[http] %d  response: %s\n", code, body.c_str());

  bool ok = (body.indexOf("\"success\":true")  >= 0 ||
             body.indexOf("\"success\": true") >= 0);
  if (!ok) {
    Serial.println("[http] No success:true in response — blink 3x");
    _httpBlink(3);
  }
  return ok;
}
