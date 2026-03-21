# otacore

Reference sketch and source for `ota_core.h` — a header-only WiFi + OTA module for ESP32.

`otacore.ino` is the minimal demo showing how to integrate `ota_core.h`. The header itself is the reusable asset: copy it into any new ESP32 sketch folder to add OTA support with three lines of code.

## Hardware

| Item | Details |
|---|---|
| Board | ESP32 WROOM (3.3V logic) |
| Onboard LED | GPIO2 — optional; enables status blinks when `OTA_LED_PIN` is defined |
| Extra hardware | None |

## OTA Firmware Updates

**Supported.** This sketch is itself OTA-enabled.

```bash
bash bash/deploy.sh otacore
```

## Using `ota_core.h` in a New Sketch

Copy `ota_core.h` and `secrets.h` into your sketch folder, then:

```cpp
#define OTA_LED_PIN 2   // optional — remove if no LED wired
#include "ota_core.h"

void setup() {
  pinMode(OTA_LED_PIN, OUTPUT);
  otaCoreSetup();
}

void loop() {
  otaCoreHandle();
  if (!otaCoreReady()) return;
  // your sketch logic here
}
```

Or use `bash/new_sketch.sh --name mysketch --board esp32 --ota` to scaffold a new sketch with `ota_core.h` pre-copied.

### Optional config (define before the include)

| Define | Default | Effect |
|---|---|---|
| `OTA_LED_PIN` | _(none)_ | GPIO for status LED blink |
| `OTA_WIFI_TIMEOUT_MS` | `15000` | WiFi connect timeout (ms) |
| `OTA_RECONNECT_MS` | `5000` | Reconnect retry interval (ms) |

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
```
