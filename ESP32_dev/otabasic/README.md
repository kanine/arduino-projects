# otabasic

Minimal WiFi + OTA bringup sketch. Connects to WiFi, starts ArduinoOTA, and holds — ready to receive wireless firmware uploads. Reconnects automatically if WiFi drops.

Use this as a recovery sketch: flash it over USB to a board that has lost OTA capability, then switch back to OTA-based deploys.

## Hardware

| Item | Details |
|---|---|
| Board | ESP32 WROOM (3.3V logic) |
| Onboard LED | GPIO2 — blinks on OTA activity (2 = ready, 3 = complete, 5 = error) |
| Extra hardware | None |

## OTA Firmware Updates

**Supported.** Once running and connected to WiFi, the board advertises itself on the network under the hostname set in `secrets.h`.

```bash
bash bash/deploy.sh otabasic
```

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
── OTA Basic ────────────────────────────────────────
Hostname: esp32-ota
Connecting to SSID: MyNetwork ....
Connected. IP: 192.168.1.254
OTA ready. Hostname: esp32-ota
           IP      : 192.168.1.254
```
