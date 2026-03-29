#pragma once

// ── Hardware ──────────────────────────────────────────────────────────────────
#define STATUS_LED_PIN         2
#define SPEAKER_PIN            4
#define I2C_SDA_PIN            21
#define I2C_SCL_PIN            22

// ── Network / timing ──────────────────────────────────────────────────────────
#define NTP_SERVER             "pool.ntp.org"
#define POST_TIMEOUT_MS        5000UL
#define WIFI_RECONNECT_MS      5000UL
#define NTP_RETRY_MS           10000UL
#define SENSOR_RETRY_MS        500UL

// ── Webhook test mode ────────────────────────────────────────────────────────
// When true, only validate that webhook replies with HTTP 2xx and "success": true.
// Runtime config updates from response bodies are ignored.
#define WEBHOOK_TEST_MODE      true

// ── Polling ───────────────────────────────────────────────────────────────────
#define POLLS_PER_MINUTE       300.0f
#define BATCH_CAP              50

// ── Sensor defaults ───────────────────────────────────────────────────────────
#define SENSOR_DISTANCE_MODE        "Short"
#define SENSOR_TIMING_BUDGET_MS     100
#define SENSOR_INTER_MEASUREMENT_MS 200
#define SENSOR_ROI_WIDTH            4
#define SENSOR_ROI_HEIGHT           4
#define SENSOR_ROI_CENTER           199

// ── Buffering ─────────────────────────────────────────────────────────────────
// Runtime buffering is guaranteed up to batch_cap * 2, bounded by this hard cap.
#define MAX_BUFFERED_READINGS  200
