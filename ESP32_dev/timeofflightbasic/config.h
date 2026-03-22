#pragma once

// ── Polling ───────────────────────────────────────────────────────────────────
// How many sensor readings to take per minute.
// The VL53L1X short-mode floor is 20 ms, so the effective maximum is 3000.
// Any value that would produce an interval below 20 ms is clamped to 20 ms.
#define POLLS_PER_MINUTE 30.0f

// ── Batching ──────────────────────────────────────────────────────────────────
// How many seconds of readings to accumulate before posting to the webhook.
// Fractional values are supported: 30.0 = 30 s, 120.0 = 2 min.
#define WINDOW_SECONDS 30.0f
