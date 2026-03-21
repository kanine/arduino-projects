# Sketch Design Philosophy

Guidelines for deciding what logic belongs in a sketch versus on the server.

---

## Core principle

Sketches have one job: **sense, batch, ship**. Keep them simple and focused.
Do the heavy lifting server-side where you have context, history, and the
ability to change logic without a recompile and OTA deploy.

---

## What belongs in the sketch

Only logic that requires an **immediate local response** — things that cannot
wait for a server round-trip, or that must function without a network connection.

### Timing and sampling
- Poll rate and batch interval live in `config.h` so they can be tuned without
  touching sketch logic
- Derived constants (e.g. poll interval in ms) are computed at compile time from
  `config.h` values — never hardcoded in loop logic

### Basic sanity checks
- Sensor not found / returns zero — hardware fault, worth a local alert
- Extreme distance thresholds: hard floor and ceiling defined in `config.h`
  (e.g. `ALARM_MIN_MM`, `ALARM_MAX_MM`) — values outside this range trigger
  a local alarm immediately, without waiting for the server

### Connectivity watchdog
- Lost WiFi is already handled by `ota_core.h` reconnect logic
- A local alarm state (LED, buzzer) can be added for prolonged disconnection
  where the server cannot be reached at all

### What not to put in the sketch
- Anything that requires **historical context** (trends, moving averages,
  anomaly detection relative to past readings)
- Statistical summaries (min, max, mean, trimmed mean) — these need the full
  dataset and can always be recomputed server-side
- Business logic or thresholds that are likely to change — recompile + OTA
  is a higher cost than a server-side config change

---

## What belongs on the server

### Statistical processing
Computed on receipt from the raw readings array in each batch:

| Stat | Notes |
|---|---|
| Minimum | Lowest reading in the batch |
| Maximum | Highest reading in the batch |
| Mean | Straight average of all readings |
| Trimmed mean | Sort readings, discard bottom and top 10% by count, average the rest |

Raw readings are always stored alongside derived stats. Pre-computed summaries
cannot be recovered into raw data — the inverse is always possible.

### Anomaly detection
- Whether a reading is anomalous **relative to recent history** requires
  context the sketch does not have
- Alarm triggers based on trends, sustained drift, or statistical outliers
  are evaluated server-side and can be acted on via a server-to-device
  mechanism or external notification

### Alarm escalation
- Immediate hardware-level alarms (buzzer, LED) fire on the device for
  threshold breaches that need instant response
- Higher-level alarms (notifications, logging, escalation) are server
  responsibility — they have the history and context to avoid false positives

---

## Sketch architecture pattern

```
config.h          — user-tunable parameters (rates, thresholds, pins)
ota_core.h        — WiFi + OTA, connectivity watchdog
http_core.h       — HTTPS POST to webhook
<sketch>.ino      — sensor init, poll loop, batch send, local alarm check
```

`loop()` structure:
1. Handle OTA
2. Guard on `otaCoreReady()` — do nothing offline except local alarm state
3. Poll sensor on `POLL_MS` interval — store reading if valid
4. Check local alarm thresholds — fire immediately if breached
5. Send batch on `BATCH_MS` interval — ship raw readings, reset buffer

---

## Rationale

- **Bandwidth is not the constraint** — over local WiFi, sending raw readings
  costs nothing. Summarising before sending would only make sense on
  bandwidth-limited transports (cellular, LoRa)
- **Flexibility** — server-side stat logic can change without touching the
  device. A recompile + OTA deploy is a meaningful cost
- **Auditability** — raw readings are the ground truth. Derived stats can
  always be recomputed; lost raw data cannot be recovered
- **Simplicity** — a sketch that only samples and ships is easier to debug,
  test, and reason about than one that also does statistics
