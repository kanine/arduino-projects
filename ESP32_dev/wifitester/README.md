# wifitester

WiFi connectivity and HTTP/HTTPS endpoint tester. Connects to a network, syncs time via NTP, then polls a configurable URL on a set interval with the current Unix timestamp appended as `?t=<timestamp>`.

Useful for verifying WiFi reachability, DNS resolution, TCP connectivity, and HTTP response from a target server — all logged to serial.

## Hardware

| Item | Details |
|---|---|
| Board | ESP32 WROOM (3.3V logic) |
| Onboard LED | GPIO2 — blinks on each HTTP request (1 = sent, 2 = success, 3 = error) |
| Extra hardware | None |

## OTA Firmware Updates

Not supported. Upload via USB only.

## Configuration

Copy `secrets.h.example` to `secrets.h` and fill in:

| Define | Description |
|---|---|
| `WIFI_SSID` | WiFi network name |
| `WIFI_PASSWORD` | WiFi password |
| `POLL_URL` | URL to poll (http:// or https://) |
| `POLL_INTERVAL_MS` | Polling interval in milliseconds |
| `POLL_CA_CERT` | _(optional)_ PEM CA certificate for TLS verification |

## Serial Output

Baud rate: **115200**

```
── WiFi Tester ──────────────────────────────────────
Poll URL      : http://example.com/ping
Poll interval : 30000 ms
Connecting to SSID: MyNetwork ....
Connected. IP: 192.168.1.100
Syncing NTP time...
Time synced. Unix time: 1700000000
Polling: http://example.com/ping?t=1700000000
Resolved example.com -> 93.184.216.34
TCP port 80 open — proceeding with HTTP request.
HTTP 200
```
