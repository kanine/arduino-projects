# Sample Response

Response from the `/batch` endpoint on successful reading submission.

```json
{
    "success": true,
    "batch_id": 634,
    "host": "esp32-ota",
    "received": 15,
    "config": {
        "polls_per_minute": 30,
        "window_seconds": 2
    },
    "actions": []
}
```

## Fields

| Field | Type | Description |
|---|---|---|
| `success` | bool | `true` if the batch was accepted |
| `batch_id` | int | Server-assigned ID for this batch |
| `host` | string | Hostname of the device as recognized by the server |
| `received` | int | Number of readings accepted in this batch |
| `config.polls_per_minute` | int | Polling rate the device should use |
| `config.window_seconds` | int | Averaging window the device should apply |
| `actions` | array | Commands for the device to act on (empty if none) |
