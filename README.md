# Arduino Projects Monorepo

A collection of self-contained Arduino sketches for **ESP32** and **Arduino Mega 2560**, with a focus on clean code patterns, wireless OTA firmware updates, and a reproducible CLI-based deploy workflow.

> All projects are works in progress. A `tested.ok` marker in the folder name or project listing indicates the sketch has been verified on real hardware.

---

## What's in this repo

### ESP32

| Sketch | Description | OTA |
|---|---|---|
| [otablink](ESP32_dev/otablink/) | WiFi + OTA bringup — confirms connectivity with a 3 s LED hold then 1 Hz blink | ✅ |
| [otabasic](ESP32_dev/otabasic/) | Minimal WiFi + OTA server — use to recover a board or as a base for new projects | ✅ |
| [otacore](ESP32_dev/otacore/) | Reference sketch and source for `ota_core.h`, a reusable OTA module | ✅ |
| [wifitester](ESP32_dev/wifitester/) | Connects to WiFi, syncs NTP, polls an HTTP/HTTPS endpoint on a timer | ❌ |
| [sketch_mar20a](ESP32_dev/sketch_mar20a/) | Hello-world — prints to serial every second | ❌ |

### Mega 2560

| Sketch | Description | Tested |
|---|---|---|
| [sonic-mm-display](Mega_2560/4digits/sonic-mm-display/) | HC-SR04 ultrasonic sensor → 4-digit 7-segment display, mm readout | — |
| [sonic-mm-detect](Mega_2560/4digits/sonic-mm-detect/) | HC-SR04 detection with LED timeout | ✅ |
| [sonic-mm-detect-light](Mega_2560/4digits/sonic-mm-detect-light/) | E18-D80NK IR sensor variant | wip |
| [counter-00-99](Mega_2560/4digits/counter-00-99/) | Button-controlled 0–99 counter on 4-digit display | ✅ |
| [counter-0-9](Mega_2560/4digits/counter-0-9/) | Single-digit button counter | — |

---

## OTA Firmware Updates (ESP32)

All ESP32 sketches that include `ota_core.h` support wireless firmware uploads via **ArduinoOTA**. No USB cable needed after the first flash.

### First flash (USB)

> **New to Arduino or connecting a board for the first time?** [Arduino IDE 2](https://www.arduino.cc/en/software) is the best starting point. It handles driver installation, board detection, and port selection through a GUI — much friendlier than the CLI for initial hardware bringup or if you're new to microcontroller programming.

```bash
arduino-cli upload --fqbn esp32:esp32:esp32 -p /dev/ttyUSB0 ESP32_dev/otablink
```

### All subsequent uploads (OTA over WiFi)

```bash
bash bash/deploy.sh otablink
```

The deploy script auto-discovers the board on the network, compiles, and uploads — one command from anywhere in the repo.

### How it works

When the board boots it connects to WiFi and calls `ArduinoOTA.begin()`. The Arduino IDE and `arduino-cli` detect it via mDNS and can push a new binary wirelessly. If WiFi drops, the sketch reconnects automatically and re-registers for OTA.

---

## Reusable OTA Module (`ota_core.h`)

[`ESP32_dev/otacore/ota_core.h`](ESP32_dev/otacore/ota_core.h) is a header-only drop-in that handles WiFi connection, reconnection, and ArduinoOTA — copy it into any sketch folder and add three lines:

```cpp
#define OTA_LED_PIN 2        // optional status LED
#include "ota_core.h"

void setup() { otaCoreSetup(); }
void loop()  { otaCoreHandle(); if (!otaCoreReady()) return; /* your code */ }
```

Credentials and hostname come from `secrets.h` (gitignored, copy from `secrets.h.example`).

---

## Creating a New Sketch

```bash
# Guided interactive prompt
bash bash/new_sketch.sh

# Or fully non-interactive
bash bash/new_sketch.sh --name myproject --board esp32 --ota
bash bash/new_sketch.sh --name myproject --board mega --category 4digits
```

This scaffolds the folder, skeleton `.ino`, `secrets.h`, `ota_core.h` (if OTA), and a `docs/wiring.md` stub in ATN-IO v3 format.

---

## Project Structure

```
arduino-projects/
  ESP32_dev/
    otacore/            # ota_core.h source + reference sketch
    otablink/           # OTA bringup + blink demo
    otabasic/           # minimal OTA server / recovery sketch
    wifitester/         # HTTP/HTTPS endpoint tester
    sketch_mar20a/      # hello-world
  Mega_2560/
    4digits/
      sonic-mm-display/ # HC-SR04 → 7-seg display
      sonic-mm-detect/  # HC-SR04 detect + LED
      counter-00-99/    # button counter
      counter-0-9/      # single-digit counter
  bash/
    deploy.sh           # compile + upload (OTA or USB)
    new_sketch.sh       # scaffold a new sketch
    sync_to_ide.sh      # sync files to Arduino IDE directory
  docs/
    wiring-notation.md  # ATN-IO v3 wiring doc format spec
```

Each sketch is fully self-contained — no cross-folder dependencies.

---

## Deploy Script

`bash/deploy.sh` wraps `arduino-cli` to handle compile and upload in one command. Board family and upload method are inferred from the sketch path.

```bash
bash bash/deploy.sh <sketch_name>              # sync + compile + upload
bash bash/deploy.sh <sketch_name> --no-sync    # skip Arduino IDE sync
bash bash/deploy.sh <sketch_name> --no-upload  # compile only
bash bash/deploy.sh <sketch_name> --port <ip>  # override upload target
```

Copy `bash/.env.example` to `bash/.env` and set your network and port config before first use.

---

## Requirements

> **Environment:** This project was developed on **Windows with WSL2 (Ubuntu) and VS Code** with the Claude Code CLI. All shell scripts and `arduino-cli` commands run inside WSL. The workflow is equally compatible with native Linux or macOS — no Windows-specific tools are required.

### arduino-cli

```bash
# Install (downloads binary to ./bin/arduino-cli)
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh

# Add to PATH — pick one:
sudo mv bin/arduino-cli /usr/local/bin/          # system-wide (any distro)
echo 'export PATH="$HOME/arduino-local/bin:$PATH"' >> ~/.profile  # user only (adjust path as needed)

# ESP32 core
arduino-cli config add board_manager.additional_urls \
  https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
arduino-cli core update-index
arduino-cli core install esp32:esp32

# Mega core
arduino-cli core install arduino:avr
```

### Arduino IDE 2

Open any `.ino` file directly. Use `bash/sync_to_ide.sh <sketch_name>` to push files from this repo to your IDE sketchbook directory first.

---

## Coding Standards

- `millis()`-based scheduling and state machines — no `delay()` in normal control flow
- `const`/`constexpr` for configuration values
- Sections separated by `// ── Section ──` banner comments
- User-tunable parameters at the top of the file under `// ── User parameters`
- Every project has a `docs/wiring.md` in [ATN-IO v3](docs/wiring-notation.md) format

---

## Wiring Documentation

Each project includes a `docs/wiring.md` describing the full electrical schematic in [ATN-IO v3](docs/wiring-notation.md) format — a plain-text notation for pin assignments and wiring paths.
