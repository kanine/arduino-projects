# otablink

WiFi + OTA connectivity test with a visual ready indicator. On boot, connects to WiFi and starts OTA. Once ready, holds the onboard LED on for 3 seconds as a confirmation signal, then blinks once per second — the traditional Arduino blink pattern — to show the sketch is running.

Use this to verify a board is on the network, OTA is functional, and the deploy workflow is working end-to-end before building a real application on top.

## Hardware

| Item | Details |
|---|---|
| Board | ESP32 WROOM (3.3V logic) |
| Onboard LED | GPIO2 — steady 3 s on boot (ready confirmation), then 1 Hz blink |
| Extra hardware | None |

## OTA Firmware Updates

**Supported.** Uses `ota_core.h` for WiFi management and OTA.

```bash
bash bash/deploy.sh otablink
```

## LED Behaviour

| Phase | LED pattern |
|---|---|
| Connecting to WiFi | Off |
| OTA initialising | 2 quick blinks (from `ota_core.h`) |
| Ready confirmation | Solid on for 3 seconds |
| Running | 1 blink per second |
| OTA upload in progress | Solid on |
| OTA complete | 3 quick blinks |
| OTA error | 5 quick blinks |

## Configuration

Copy `secrets.h.example` to `secrets.h` and fill in:

| Define | Description |
|---|---|
| `WIFI_SSID` | WiFi network name |
| `WIFI_PASSWORD` | WiFi password |
| `OTA_HOSTNAME` | mDNS hostname shown in Arduino IDE network ports |
| `OTA_PASSWORD` | _(optional)_ Password to protect OTA uploads — comment out to disable |

## Serial Output

Baud rate: **115200**

```
[ota] Connecting to MyNetwork .... IP: 192.168.1.254
[ota] Ready — hostname: esp32-ota  IP: 192.168.1.254
[otablink] Ready.
```
