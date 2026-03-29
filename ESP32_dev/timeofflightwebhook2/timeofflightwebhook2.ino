// timeofflightwebhook2 – ESP32
//
// Standalone ESP32 firmware for a VL53L1X time-of-flight sensor using the
// Pololu VL53L1X library. Valid readings are batched and sent to WEBHOOK_URL.

#include <ArduinoOTA.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <VL53L1X.h>
#include <sys/time.h>
#include <time.h>

#include "config.h"
#include "secrets.h"

#ifndef DEVICE_HOST
#define DEVICE_HOST OTA_HOSTNAME
#endif

constexpr uint8_t SENSOR_INT_CLEAR_REG = 0x86;
constexpr uint32_t STARTUP_ON_MS = 300;
constexpr uint32_t STARTUP_OFF_PHASE1_MS = 300;
constexpr uint32_t STARTUP_OFF_MS = 200;
constexpr uint32_t STARTUP_PHASE_PAUSE_MS = 500;
constexpr uint32_t LED_FLASH_MS = 120;
constexpr uint32_t LED_HOLD_MS = 500;
constexpr uint32_t SENSOR_TIMEOUT_MS = 250;
constexpr time_t VALID_UNIX_TIME_THRESHOLD = 1000000000UL;

enum LedMode : uint8_t {
  LED_MODE_IDLE,
  LED_MODE_PULSE,
  LED_MODE_TRIPLE_FLASH,
  LED_MODE_HOLD
};

struct PollingConfig {
  float pollsPerMinute;
  uint16_t batchCap;
};

struct SensorConfig {
  VL53L1X::DistanceMode distanceMode;
  uint16_t timingBudgetMs;
  uint16_t interMeasurementMs;
  uint8_t roiWidth;
  uint8_t roiHeight;
  uint8_t roiCenter;
};

struct Reading {
  uint64_t tsMs;
  uint16_t distanceMm;
  uint8_t rangeStatus;
  float signalMcps;
  float ambientMcps;
};

struct PostResult {
  bool transportOk;
  bool success;
  int statusCode;
  String body;
};

static PollingConfig pollingConfig = { POLLS_PER_MINUTE, BATCH_CAP };
static SensorConfig sensorConfig;
static Reading readings[MAX_BUFFERED_READINGS];
static uint16_t readingCount = 0;
static uint32_t batchId = 0;
static uint32_t pollMs = 200;
static uint32_t lastPollAt = 0;
static uint32_t lastWiFiAttemptAt = 0;
static uint32_t lastNtpRetryAt = 0;
static uint32_t lastSensorInitAt = 0;
static bool timeSynced = false;
static bool sensorReady = false;
static bool otaReady = false;
static wl_status_t lastWiFiStatus = WL_IDLE_STATUS;
static LedMode ledMode = LED_MODE_IDLE;
static bool ledIsOn = false;
static uint8_t ledFlashesRemaining = 0;
static uint32_t ledDeadline = 0;
static VL53L1X sensor;

static uint32_t clampU32(uint32_t value, uint32_t lower, uint32_t upper) {
  if (value < lower) return lower;
  if (value > upper) return upper;
  return value;
}

static uint8_t clampU8(uint8_t value, uint8_t lower, uint8_t upper) {
  if (value < lower) return lower;
  if (value > upper) return upper;
  return value;
}

static uint16_t currentBufferLimit() {
  uint32_t limit = (uint32_t)pollingConfig.batchCap * 2U;
  if (limit == 0) limit = 1;
  if (limit > MAX_BUFFERED_READINGS) limit = MAX_BUFFERED_READINGS;
  return (uint16_t)limit;
}

static void recomputePolling() {
  if (pollingConfig.pollsPerMinute <= 0.0f) {
    pollingConfig.pollsPerMinute = 1.0f;
  }
  if (pollingConfig.batchCap == 0) {
    pollingConfig.batchCap = 1;
  }
  if (pollingConfig.batchCap > (MAX_BUFFERED_READINGS / 2U)) {
    pollingConfig.batchCap = MAX_BUFFERED_READINGS / 2U;
  }

  float rawPoll = 60000.0f / pollingConfig.pollsPerMinute;
  if (rawPoll < 20.0f) rawPoll = 20.0f;
  pollMs = (uint32_t)(rawPoll + 0.5f);
}

static const char* distanceModeName(VL53L1X::DistanceMode mode) {
  switch (mode) {
    case VL53L1X::Short: return "Short";
    case VL53L1X::Medium: return "Medium";
    case VL53L1X::Long: return "Long";
    default: return "Unknown";
  }
}

static bool parseDistanceMode(const String& value, VL53L1X::DistanceMode* mode) {
  if (value == "Short") {
    *mode = VL53L1X::Short;
    return true;
  }
  if (value == "Medium") {
    *mode = VL53L1X::Medium;
    return true;
  }
  if (value == "Long") {
    *mode = VL53L1X::Long;
    return true;
  }
  return false;
}

static bool isValidTimingBudget(uint16_t timingBudgetMs) {
  switch (timingBudgetMs) {
    case 15:
    case 20:
    case 33:
    case 50:
    case 100:
    case 200:
    case 500:
      return true;
    default:
      return false;
  }
}

static bool isTimingBudgetSupportedForMode(VL53L1X::DistanceMode mode, uint16_t timingBudgetMs) {
  if (!isValidTimingBudget(timingBudgetMs)) return false;
  if (mode == VL53L1X::Short) {
    return timingBudgetMs >= 20;
  }
  return timingBudgetMs >= 33;
}

static void setLedHardware(bool on) {
  digitalWrite(STATUS_LED_PIN, on ? HIGH : LOW);
  ledIsOn = on;
}

static void triggerSuccessFlash() {
  ledMode = LED_MODE_PULSE;
  ledFlashesRemaining = 1;
  setLedHardware(true);
  ledDeadline = millis() + LED_FLASH_MS;
}

static void triggerFailureFlash() {
  ledMode = LED_MODE_TRIPLE_FLASH;
  ledFlashesRemaining = 3;
  setLedHardware(true);
  ledDeadline = millis() + LED_FLASH_MS;
}

static void triggerConfigAppliedHold() {
  ledMode = LED_MODE_HOLD;
  setLedHardware(true);
  ledDeadline = millis() + LED_HOLD_MS;
}

static void handleStatusLed() {
  if (ledMode == LED_MODE_IDLE) return;

  uint32_t now = millis();
  if ((int32_t)(now - ledDeadline) < 0) return;

  if (ledMode == LED_MODE_PULSE) {
    setLedHardware(false);
    ledMode = LED_MODE_IDLE;
    return;
  }

  if (ledMode == LED_MODE_HOLD) {
    setLedHardware(false);
    ledMode = LED_MODE_IDLE;
    return;
  }

  if (ledMode == LED_MODE_TRIPLE_FLASH) {
    if (ledIsOn) {
      setLedHardware(false);
      ledDeadline = now + LED_FLASH_MS;
      return;
    }

    if (ledFlashesRemaining > 1) {
      ledFlashesRemaining--;
      setLedHardware(true);
      ledDeadline = now + LED_FLASH_MS;
    } else {
      ledFlashesRemaining = 0;
      ledMode = LED_MODE_IDLE;
    }
  }
}

static void blinkBlocking(uint8_t count, uint32_t offMs) {
  for (uint8_t i = 0; i < count; i++) {
    digitalWrite(STATUS_LED_PIN, HIGH);
    delay(STARTUP_ON_MS);
    digitalWrite(STATUS_LED_PIN, LOW);
    delay(offMs);
  }
}

static void runStartupSequence() {
  blinkBlocking(1, STARTUP_OFF_PHASE1_MS);
  delay(STARTUP_PHASE_PAUSE_MS);
  blinkBlocking(2, STARTUP_OFF_MS);
  delay(STARTUP_PHASE_PAUSE_MS);
  blinkBlocking(3, STARTUP_OFF_MS);
  delay(STARTUP_PHASE_PAUSE_MS);
}

static uint64_t unixTimeMs() {
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  return (uint64_t)tv.tv_sec * 1000ULL + (uint64_t)(tv.tv_usec / 1000ULL);
}

static void formatUptime(char* buffer, size_t size) {
  uint32_t uptimeSeconds = millis() / 1000UL;
  uint32_t days = uptimeSeconds / 86400UL;
  uint32_t hours = (uptimeSeconds % 86400UL) / 3600UL;
  uint32_t minutes = (uptimeSeconds % 3600UL) / 60UL;
  uint32_t seconds = uptimeSeconds % 60UL;
  snprintf(buffer, size, "%lud %02lu:%02lu:%02lu",
           (unsigned long)days,
           (unsigned long)hours,
           (unsigned long)minutes,
           (unsigned long)seconds);
}

static void clearSensorInterrupt() {
  sensor.writeReg(SENSOR_INT_CLEAR_REG, 0x01);
}

static bool applySensorConfigToHardware(const SensorConfig& config) {
  sensor.stopContinuous();

  if (!sensor.setDistanceMode(config.distanceMode)) {
    Serial.println("[tof] failed to set distance mode");
    return false;
  }
  if (!sensor.setMeasurementTimingBudget((uint32_t)config.timingBudgetMs * 1000UL)) {
    Serial.println("[tof] failed to set timing budget");
    return false;
  }

  sensor.setROISize(config.roiWidth, config.roiHeight);
  sensor.setROICenter(config.roiCenter);
  sensor.startContinuous(config.interMeasurementMs);
  clearSensorInterrupt();
  return true;
}

static void loadDefaultSensorConfig() {
  if (!parseDistanceMode(String(SENSOR_DISTANCE_MODE), &sensorConfig.distanceMode)) {
    sensorConfig.distanceMode = VL53L1X::Short;
  }
  sensorConfig.timingBudgetMs = SENSOR_TIMING_BUDGET_MS;
  sensorConfig.interMeasurementMs = SENSOR_INTER_MEASUREMENT_MS;
  sensorConfig.roiWidth = clampU8(SENSOR_ROI_WIDTH, 4, 16);
  sensorConfig.roiHeight = clampU8(SENSOR_ROI_HEIGHT, 4, 16);
  sensorConfig.roiCenter = SENSOR_ROI_CENTER;
}

static void beginWiFiConnect() {
  Serial.printf("[wifi] connecting to %s\n", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  lastWiFiAttemptAt = millis();
}

static void beginOta() {
  ArduinoOTA.setHostname(OTA_HOSTNAME);
#ifdef OTA_PASSWORD
  ArduinoOTA.setPassword(OTA_PASSWORD);
#endif
  ArduinoOTA.onStart([]() {
    Serial.println("[ota] start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("[ota] complete");
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("[ota] error: %u\n", (unsigned)error);
  });
  ArduinoOTA.begin();
  otaReady = true;
  Serial.printf("[ota] ready on %s\n", OTA_HOSTNAME);
}

static void handleWiFi() {
  wl_status_t status = WiFi.status();
  if (status != lastWiFiStatus) {
    if (status == WL_CONNECTED) {
      Serial.printf("[wifi] connected ip=%s\n", WiFi.localIP().toString().c_str());
      beginOta();
    } else if (lastWiFiStatus == WL_CONNECTED) {
      Serial.println("[wifi] disconnected");
      otaReady = false;
    }
    lastWiFiStatus = status;
  }

  if (status != WL_CONNECTED) {
    uint32_t now = millis();
    if (now - lastWiFiAttemptAt >= WIFI_RECONNECT_MS) {
      WiFi.disconnect();
      beginWiFiConnect();
    }
    return;
  }

  if (otaReady) {
    ArduinoOTA.handle();
  }
}

static void requestNtpSync() {
  configTime(0, 0, NTP_SERVER);
  lastNtpRetryAt = millis();
  Serial.printf("[time] requesting NTP sync from %s\n", NTP_SERVER);
}

static void handleTimeSync() {
  if (timeSynced || WiFi.status() != WL_CONNECTED) return;

  time_t now = time(nullptr);
  if (now >= VALID_UNIX_TIME_THRESHOLD) {
    timeSynced = true;
    Serial.println("[time] NTP sync complete");
    return;
  }

  uint32_t nowMs = millis();
  if (nowMs - lastNtpRetryAt >= NTP_RETRY_MS) {
    requestNtpSync();
  }
}

static bool tryInitSensor() {
  sensor.setBus(&Wire);
  sensor.setTimeout(SENSOR_TIMEOUT_MS);
  if (!sensor.init()) {
    return false;
  }
  if (!applySensorConfigToHardware(sensorConfig)) {
    return false;
  }
  sensorReady = true;
  Serial.printf("[tof] ready mode=%s timing=%ums inter=%ums roi=%ux%u center=%u\n",
                distanceModeName(sensorConfig.distanceMode),
                sensorConfig.timingBudgetMs,
                sensorConfig.interMeasurementMs,
                sensorConfig.roiWidth,
                sensorConfig.roiHeight,
                sensorConfig.roiCenter);
  return true;
}

static void handleSensorInit() {
  if (sensorReady) return;
  uint32_t now = millis();
  if (now - lastSensorInitAt < SENSOR_RETRY_MS) return;
  lastSensorInitAt = now;
  if (!tryInitSensor()) {
    Serial.println("[tof] sensor not ready; retrying");
  }
}

static int skipWhitespace(const String& text, int index) {
  while (index < text.length()) {
    char c = text[index];
    if (c != ' ' && c != '\n' && c != '\r' && c != '\t') break;
    index++;
  }
  return index;
}

static bool findObjectSpan(const String& text, const char* key, int* objectStart, int* objectEnd) {
  int keyIndex = text.indexOf(key);
  if (keyIndex < 0) return false;
  int colon = text.indexOf(':', keyIndex + (int)strlen(key));
  if (colon < 0) return false;
  int start = skipWhitespace(text, colon + 1);
  if (start >= text.length() || text[start] != '{') return false;

  int depth = 0;
  bool inString = false;
  for (int i = start; i < text.length(); i++) {
    char c = text[i];
    if (c == '"' && (i == 0 || text[i - 1] != '\\')) {
      inString = !inString;
    }
    if (inString) continue;
    if (c == '{') depth++;
    if (c == '}') {
      depth--;
      if (depth == 0) {
        *objectStart = start;
        *objectEnd = i;
        return true;
      }
    }
  }
  return false;
}

static bool extractStringValue(const String& text, const char* key, String* value) {
  int keyIndex = text.indexOf(key);
  if (keyIndex < 0) return false;
  int colon = text.indexOf(':', keyIndex + (int)strlen(key));
  if (colon < 0) return false;
  int start = skipWhitespace(text, colon + 1);
  if (start >= text.length() || text[start] != '"') return false;
  start++;
  int end = start;
  while (end < text.length()) {
    if (text[end] == '"' && text[end - 1] != '\\') break;
    end++;
  }
  if (end >= text.length()) return false;
  *value = text.substring(start, end);
  return true;
}

static bool extractFloatValue(const String& text, const char* key, float* value) {
  int keyIndex = text.indexOf(key);
  if (keyIndex < 0) return false;
  int colon = text.indexOf(':', keyIndex + (int)strlen(key));
  if (colon < 0) return false;
  int start = skipWhitespace(text, colon + 1);
  int end = start;
  while (end < text.length()) {
    char c = text[end];
    if ((c < '0' || c > '9') && c != '-' && c != '+' && c != '.') break;
    end++;
  }
  if (end == start) return false;
  *value = text.substring(start, end).toFloat();
  return true;
}

static bool extractUIntValue(const String& text, const char* key, uint32_t* value) {
  float parsed = 0.0f;
  if (!extractFloatValue(text, key, &parsed)) return false;
  if (parsed < 0.0f) return false;
  *value = (uint32_t)parsed;
  return true;
}

static bool responseSuccessTrue(const String& body) {
  return body.indexOf("\"success\":true") >= 0 ||
         body.indexOf("\"success\": true") >= 0;
}

static void dropFirstReadings(uint16_t count) {
  if (count >= readingCount) {
    readingCount = 0;
    return;
  }
  uint16_t remaining = readingCount - count;
  memmove(readings, readings + count, remaining * sizeof(Reading));
  readingCount = remaining;
}

static bool enqueueReading(const Reading& reading) {
  uint16_t limit = currentBufferLimit();
  if (readingCount >= limit) {
    Serial.printf("[main] buffer full (%u) dropping reading\n", limit);
    return false;
  }
  readings[readingCount++] = reading;
  return true;
}

static PostResult postJson(const char* json) {
  PostResult result = { false, false, 0, String() };
  HTTPClient http;
  bool https = String(WEBHOOK_URL).startsWith("https://");

  if (https) {
    WiFiClientSecure client;
#ifdef HTTP_CA_CERT
    client.setCACert(HTTP_CA_CERT);
#else
    client.setInsecure();
#endif
    if (!http.begin(client, WEBHOOK_URL)) {
      Serial.println("[http] begin failed");
      return result;
    }
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(POST_TIMEOUT_MS);
    result.statusCode = http.POST((uint8_t*)json, strlen(json));
    result.transportOk = result.statusCode > 0;
    if (result.transportOk) {
      result.body = http.getString();
    }
    http.end();
  } else {
    WiFiClient client;
    if (!http.begin(client, WEBHOOK_URL)) {
      Serial.println("[http] begin failed");
      return result;
    }
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(POST_TIMEOUT_MS);
    result.statusCode = http.POST((uint8_t*)json, strlen(json));
    result.transportOk = result.statusCode > 0;
    if (result.transportOk) {
      result.body = http.getString();
    }
    http.end();
  }

  result.success = result.transportOk &&
                   result.statusCode >= 200 &&
                   result.statusCode < 300 &&
                   responseSuccessTrue(result.body);
  return result;
}

static bool applyRuntimeConfig(const String& body) {
  int configStart = 0;
  int configEnd = 0;
  if (!findObjectSpan(body, "\"config\"", &configStart, &configEnd)) {
    return false;
  }

  String configBlock = body.substring(configStart, configEnd + 1);
  bool changed = false;

  int pollingStart = 0;
  int pollingEnd = 0;
  if (findObjectSpan(configBlock, "\"polling\"", &pollingStart, &pollingEnd)) {
    String pollingBlock = configBlock.substring(pollingStart, pollingEnd + 1);
    PollingConfig candidate = pollingConfig;
    uint32_t intValue = 0;
    float floatValue = 0.0f;
    bool pollingChanged = false;

    if (extractFloatValue(pollingBlock, "\"polls_per_minute\"", &floatValue) && floatValue > 0.0f) {
      candidate.pollsPerMinute = floatValue;
      pollingChanged = true;
    }
    if (extractUIntValue(pollingBlock, "\"batch_cap\"", &intValue) && intValue > 0) {
      candidate.batchCap = (uint16_t)intValue;
      pollingChanged = true;
    }

    if (pollingChanged) {
      pollingConfig = candidate;
      recomputePolling();
      changed = true;
      Serial.printf("[cfg] polling ppm=%.1f poll_ms=%lu batch_cap=%u\n",
                    pollingConfig.pollsPerMinute,
                    (unsigned long)pollMs,
                    pollingConfig.batchCap);
    }
  }

  int sensorStart = 0;
  int sensorEnd = 0;
  if (findObjectSpan(configBlock, "\"sensor\"", &sensorStart, &sensorEnd)) {
    String sensorBlock = configBlock.substring(sensorStart, sensorEnd + 1);
    SensorConfig candidate = sensorConfig;
    String stringValue;
    uint32_t intValue = 0;
    bool sensorChanged = false;

    if (extractStringValue(sensorBlock, "\"distance_mode\"", &stringValue)) {
      VL53L1X::DistanceMode mode;
      if (parseDistanceMode(stringValue, &mode)) {
        candidate.distanceMode = mode;
        sensorChanged = true;
      }
    }
    if (extractUIntValue(sensorBlock, "\"timing_budget_ms\"", &intValue) && isValidTimingBudget((uint16_t)intValue)) {
      candidate.timingBudgetMs = (uint16_t)intValue;
      sensorChanged = true;
    }
    if (extractUIntValue(sensorBlock, "\"inter_measurement_ms\"", &intValue)) {
      candidate.interMeasurementMs = (uint16_t)intValue;
      sensorChanged = true;
    }
    if (extractUIntValue(sensorBlock, "\"roi_width\"", &intValue)) {
      candidate.roiWidth = clampU8((uint8_t)intValue, 4, 16);
      sensorChanged = true;
    }
    if (extractUIntValue(sensorBlock, "\"roi_height\"", &intValue)) {
      candidate.roiHeight = clampU8((uint8_t)intValue, 4, 16);
      sensorChanged = true;
    }
    if (extractUIntValue(sensorBlock, "\"roi_center\"", &intValue) && intValue <= 255U) {
      candidate.roiCenter = (uint8_t)intValue;
      sensorChanged = true;
    }

    if (sensorChanged) {
      if (candidate.interMeasurementMs < candidate.timingBudgetMs) {
        Serial.println("[cfg] rejected sensor block: inter_measurement_ms < timing_budget_ms");
      } else if (!isTimingBudgetSupportedForMode(candidate.distanceMode, candidate.timingBudgetMs)) {
        Serial.println("[cfg] rejected sensor block: timing budget not supported for distance mode");
      } else if (!sensorReady || applySensorConfigToHardware(candidate)) {
        sensorConfig = candidate;
        changed = true;
        Serial.printf("[cfg] sensor mode=%s timing=%u inter=%u roi=%ux%u center=%u\n",
                      distanceModeName(sensorConfig.distanceMode),
                      sensorConfig.timingBudgetMs,
                      sensorConfig.interMeasurementMs,
                      sensorConfig.roiWidth,
                      sensorConfig.roiHeight,
                      sensorConfig.roiCenter);
      } else {
        Serial.println("[cfg] sensor block failed to apply");
        sensorReady = false;
      }
    }
  }

  if (changed) {
    triggerConfigAppliedHold();
  }
  return changed;
}

static char* buildPayload(uint16_t sendCount) {
  char uptime[20];
  formatUptime(uptime, sizeof(uptime));

  uint64_t startTimeMs = readings[0].tsMs;
  uint64_t endTimeMs = readings[sendCount - 1].tsMs;
  size_t bufferSize = 512 + ((size_t)sendCount * 128U);
  char* json = (char*)malloc(bufferSize);
  if (!json) return nullptr;

  int pos = snprintf(
    json,
    bufferSize,
    "{\"app\":\"esp32-tof-webhook\",\"host\":\"%s\",\"batch_id\":%lu,"
    "\"start_time_ms\":%llu,\"end_time_ms\":%llu,\"uptime\":\"%s\","
    "\"config\":{\"polls_per_minute\":%.3f,\"poll_ms\":%lu,\"batch_cap\":%u,"
    "\"sensor\":{\"distance_mode\":\"%s\",\"timing_budget_ms\":%u,"
    "\"inter_measurement_ms\":%u,\"roi_width\":%u,\"roi_height\":%u,\"roi_center\":%u}},"
    "\"readings\":[",
    DEVICE_HOST,
    (unsigned long)batchId,
    (unsigned long long)startTimeMs,
    (unsigned long long)endTimeMs,
    uptime,
    pollingConfig.pollsPerMinute,
    (unsigned long)pollMs,
    pollingConfig.batchCap,
    distanceModeName(sensorConfig.distanceMode),
    sensorConfig.timingBudgetMs,
    sensorConfig.interMeasurementMs,
    sensorConfig.roiWidth,
    sensorConfig.roiHeight,
    sensorConfig.roiCenter
  );

  for (uint16_t i = 0; i < sendCount && pos > 0 && (size_t)pos < bufferSize; i++) {
    pos += snprintf(
      json + pos,
      bufferSize - (size_t)pos,
      "%s{\"ts_ms\":%llu,\"distance_mm\":%u,\"range_status\":%u,\"signal_mcps\":%.3f,\"ambient_mcps\":%.3f}",
      i == 0 ? "" : ",",
      (unsigned long long)readings[i].tsMs,
      readings[i].distanceMm,
      readings[i].rangeStatus,
      readings[i].signalMcps,
      readings[i].ambientMcps
    );
  }

  if (pos > 0 && (size_t)pos < bufferSize) {
    snprintf(json + pos, bufferSize - (size_t)pos, "]}");
    return json;
  }

  free(json);
  return nullptr;
}

static void trySendBatch() {
  if (WiFi.status() != WL_CONNECTED || !timeSynced || readingCount < pollingConfig.batchCap) {
    return;
  }

  uint16_t sendCount = pollingConfig.batchCap;
  batchId++;
  char* payload = buildPayload(sendCount);
  if (!payload) {
    Serial.println("[http] failed to allocate payload buffer");
    dropFirstReadings(sendCount);
    return;
  }

  Serial.printf("[http] sending batch %lu with %u readings\n", (unsigned long)batchId, sendCount);
  PostResult result = postJson(payload);
  free(payload);

  if (!result.transportOk) {
    Serial.println("[http] POST failed at transport layer; discarding batch");
    triggerFailureFlash();
    dropFirstReadings(sendCount);
    return;
  }

  Serial.printf("[http] status=%d body=%s\n", result.statusCode, result.body.c_str());
  if (!result.success) {
    Serial.println("[http] webhook returned failure; discarding batch");
    triggerFailureFlash();
    dropFirstReadings(sendCount);
    return;
  }

  triggerSuccessFlash();
  applyRuntimeConfig(result.body);
  dropFirstReadings(sendCount);
}

static void handleSensorPolling() {
  if (!sensorReady || !timeSynced) return;

  uint32_t now = millis();
  if (now - lastPollAt < pollMs) return;
  lastPollAt = now;

  if (!sensor.dataReady()) {
    return;
  }

  uint16_t distanceMm = sensor.read(false);
  clearSensorInterrupt();

  if (sensor.timeoutOccurred() || sensor.last_status != 0) {
    Serial.printf("[tof] I2C error timeout=%s status=%u; reinitialising\n",
                  sensor.timeoutOccurred() ? "yes" : "no",
                  sensor.last_status);
    sensorReady = false;
    return;
  }

  uint8_t rangeStatus = (uint8_t)sensor.ranging_data.range_status;
  if (rangeStatus != 0) {
    return;
  }

  Reading reading = {
    unixTimeMs(),
    distanceMm,
    rangeStatus,
    sensor.ranging_data.peak_signal_count_rate_MCPS,
    sensor.ranging_data.ambient_count_rate_MCPS
  };
  enqueueReading(reading);
}

void setup() {
  Serial.begin(115200);
  delay(1500);

  pinMode(STATUS_LED_PIN, OUTPUT);
  setLedHardware(false);

  loadDefaultSensorConfig();
  recomputePolling();

  runStartupSequence();

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  beginWiFiConnect();
  requestNtpSync();
}

void loop() {
  handleStatusLed();
  handleWiFi();
  handleTimeSync();
  handleSensorInit();
  handleSensorPolling();
  trySendBatch();
}
