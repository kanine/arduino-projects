// WiFi tester – ESP32
// Connects to WiFi, syncs time via NTP, then polls a URL every POLL_INTERVAL_MS
// with the current Unix timestamp appended as ?t=<timestamp>.
// Supports both http:// and https:// URLs.
//
// Configuration lives in secrets.h (copy from secrets.h.example).
// Required libraries (Arduino IDE Library Manager):
//   - WiFi         (bundled with ESP32 Arduino core)
//   - HTTPClient   (bundled with ESP32 Arduino core)

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <time.h>
#include "secrets.h"

// ── User parameters ───────────────────────────────────────────────────────────
static const uint8_t LED_PIN = 2;   // onboard blue LED (GPIO2, active HIGH)

// ── Constants ─────────────────────────────────────────────────────────────────
static const char* NTP_SERVER    = "pool.ntp.org";
static const long  GMT_OFFSET_S  = 0;    // UTC; adjust for local timezone if needed
static const int   DST_OFFSET_S  = 0;

static const unsigned long WIFI_TIMEOUT_MS  = 15000;
static const unsigned long NTP_TIMEOUT_MS   = 10000;
static const unsigned long RECONNECT_WAIT_MS = 5000;

// ── State ─────────────────────────────────────────────────────────────────────
static unsigned long lastPollMs    = 0;
static unsigned long lastReconnect = 0;
static bool          timeSynced    = false;

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

static bool syncNTP() {
  Serial.println("Syncing NTP time...");
  configTime(GMT_OFFSET_S, DST_OFFSET_S, NTP_SERVER);

  unsigned long start = millis();
  time_t now = 0;
  while (now < 1000000000UL) {          // wait for a plausible epoch value
    if (millis() - start >= NTP_TIMEOUT_MS) {
      Serial.println("NTP sync timed out.");
      return false;
    }
    delay(200);
    time(&now);
  }
  Serial.printf("Time synced. Unix time: %lu\n", (unsigned long)now);
  return true;
}

// Returns the port implied by the URL scheme, or 0 if unrecognised.
static uint16_t defaultPort(const String& url) {
  if (url.startsWith("https://")) return 443;
  if (url.startsWith("http://"))  return 80;
  return 0;
}

// Extracts the hostname from a URL (strips scheme and any path/query).
static String extractHost(const String& url) {
  int start = url.indexOf("://");
  if (start < 0) return url;
  start += 3;
  int end = url.indexOf('/', start);
  return (end < 0) ? url.substring(start) : url.substring(start, end);
}

static void doPoll() {
  time_t now = 0;
  time(&now);
  if (now < 1000000000UL) {
    Serial.println("Poll skipped — time not yet valid.");
    return;
  }

  String url  = String(POLL_URL) + "?t=" + String((unsigned long)now);
  String host = extractHost(url);
  uint16_t port = defaultPort(url);
  Serial.printf("Polling: %s\n", url.c_str());

  // ── Pre-flight: resolve hostname ────────────────────────────────────────
  IPAddress resolved;
  if (WiFi.hostByName(host.c_str(), resolved)) {
    Serial.printf("Resolved %s -> %s\n", host.c_str(), resolved.toString().c_str());
  } else {
    Serial.printf("DNS failed for: %s\n", host.c_str());
    blinkLed(3);
    return;
  }

  // ── Pre-flight: TCP reachability ────────────────────────────────────────
  WiFiClient probe;
  probe.setTimeout(5000);
  if (!probe.connect(resolved, port)) {
    Serial.printf("TCP connect failed on port %u — nothing listening there.\n", port);
    Serial.println("  Check: is the server running? Is port forwarding set up?");
    probe.stop();
    blinkLed(3);
    return;
  }
  probe.stop();
  Serial.printf("TCP port %u open — proceeding with HTTP request.\n", port);

  // ── HTTP / HTTPS request ─────────────────────────────────────────────────
  HTTPClient http;
  bool isHttps = url.startsWith("https://");
  WiFiClientSecure secureClient;

  if (isHttps) {
#ifdef POLL_CA_CERT
    secureClient.setCACert(POLL_CA_CERT);
    Serial.println("TLS: using CA certificate from secrets.h");
#else
    secureClient.setInsecure();
    Serial.println("TLS: no CA cert defined — skipping verification");
#endif

    // Extract path+query after the host (more reliable than passing full URL)
    int hostStart = url.indexOf("://") + 3;
    int pathStart = url.indexOf('/', hostStart);
    String path   = (pathStart >= 0) ? url.substring(pathStart) : "/";

    http.begin(secureClient, host, port, path, true);
  } else {
    http.begin(url);
  }
  http.setTimeout(10000);

  blinkLed(1);
  int code = http.GET();
  if (code > 0) {
    Serial.printf("HTTP %d\n", code);
    blinkLed(2);
    if (code == HTTP_CODE_OK) {
      Serial.println(http.getString());
    }
  } else {
    Serial.printf("HTTP error: %s\n", http.errorToString(code).c_str());
    blinkLed(3);
    if (isHttps) {
      char sslErr[256] = {0};
      secureClient.lastError(sslErr, sizeof(sslErr));
      if (strlen(sslErr) > 0) {
        Serial.printf("SSL error: %s\n", sslErr);
      }
    }
  }
  http.end();
}

// ── Setup ─────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  delay(500);

  Serial.println("\n── WiFi Tester ──────────────────────────────────────");
  Serial.printf("Poll URL      : %s\n", POLL_URL);
  Serial.printf("Poll interval : %u ms\n", POLL_INTERVAL_MS);

  if (connectWiFi()) {
    timeSynced = syncNTP();
    doPoll();
    lastPollMs = millis();
  }
}

// ── Loop ──────────────────────────────────────────────────────────────────────
void loop() {
  unsigned long now = millis();

  // ── Reconnect if dropped ────────────────────────────────────────────────
  if (WiFi.status() != WL_CONNECTED) {
    if (now - lastReconnect >= RECONNECT_WAIT_MS) {
      lastReconnect = now;
      Serial.println("WiFi lost. Reconnecting...");
      WiFi.disconnect();
      if (connectWiFi() && !timeSynced) {
        timeSynced = syncNTP();
      }
    }
    return;   // don't poll until connected
  }

  // ── Re-sync NTP once connected if first sync failed ─────────────────────
  if (!timeSynced) {
    timeSynced = syncNTP();
  }

  // ── Scheduled poll ──────────────────────────────────────────────────────
  if (now - lastPollMs >= POLL_INTERVAL_MS) {
    lastPollMs = now;
    doPoll();
  }
}
