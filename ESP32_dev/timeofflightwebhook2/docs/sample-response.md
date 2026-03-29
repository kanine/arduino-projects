# Sample Response

Minimal acknowledgement:

```json
{
    "success": true
}
```

Acknowledgement with runtime config update:

```json
{
    "success": true,
    "config": {
        "polling": {
            "polls_per_minute": 60,
            "batch_cap": 20
        },
        "sensor": {
            "distance_mode": "Long",
            "timing_budget_ms": 200,
            "inter_measurement_ms": 300,
            "roi_width": 8,
            "roi_height": 8,
            "roi_center": 199
        }
    }
}
```

Notes:

- `success` must be `true` for the device to treat the POST as successful.
- `config` is optional.
- `polling` and `sensor` are independently optional.
- Only keys present in the response are applied.
