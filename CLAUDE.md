# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Building and Uploading

### Preferred: deploy script (CLI, all-in-one)

```bash
bash bash/deploy.sh <sketch_name>          # sync + compile + upload
bash bash/deploy.sh <sketch_name> --no-sync   # skip IDE sync
bash bash/deploy.sh <sketch_name> --no-upload # compile only
bash bash/deploy.sh <sketch_name> --port <ip_or_tty>  # override port
```

If `arduino-cli: command not found`, the shell hasn't sourced `~/.bashrc`. Prefix with the explicit path:
```bash
PATH="$PATH:/home/kanine/arduino-local/bin" bash bash/deploy.sh <sketch_name> --no-sync
```

The script infers the board and upload method from the sketch path:
- `ESP32_dev/*` → `esp32:esp32:esp32`, OTA upload over network
- `Mega_2560/*` → `arduino:avr:mega`, serial upload via USB

Upload targets and FQBNs are configured in `bash/.env` (copy from `bash/.env.example`).

### Alternative: Arduino IDE 2

Open the `.ino` file directly in Arduino IDE 2, select the correct board and port, then use Sketch > Upload. Use `bash/sync_to_ide.sh <sketch_name>` to push files to the IDE directory first.

### arduino-cli directly

```bash
# ESP32 – compile
arduino-cli compile --fqbn esp32:esp32:esp32 ESP32_dev/<sketch>/

# ESP32 – OTA upload
arduino-cli upload --fqbn esp32:esp32:esp32 \
  --port <ip> --protocol network \
  --upload-field password= \
  ESP32_dev/<sketch>/

# Mega – compile + upload via USB
arduino-cli compile --fqbn arduino:avr:mega Mega_2560/<path>/<sketch>/
arduino-cli upload  --fqbn arduino:avr:mega -p /dev/ttyUSB0 Mega_2560/<path>/<sketch>/
```

Serial monitor baud rates in use: **19200** (Mega 2560 projects), **115200** (ESP32 projects).

## Creating a New Sketch

Use `bash/new_sketch.sh` to scaffold a new sketch. It creates the folder, `.ino`, `docs/wiring.md`, and optionally copies `ota_core.h` and `secrets.h`.

```bash
# Non-interactive (all args supplied — use this form when invoked by Claude):
bash bash/new_sketch.sh --name mysketch --board esp32 --ota
bash bash/new_sketch.sh --name mysketch --board esp32 --no-ota
bash bash/new_sketch.sh --name mysketch --board mega
bash bash/new_sketch.sh --name mysketch --board mega --category 4digits

# Interactive (prompts for missing values — use when run by the user directly):
bash bash/new_sketch.sh
```

When the user asks to "create a new sketch" without specifying name/board/OTA:
- Ask: sketch name, board (esp32/mega), OTA support (yes/no, ESP32 only)
- Then run the non-interactive form with all args supplied

## OTA Core (`ota_core.h`)

`ESP32_dev/otacore/ota_core.h` is a header-only WiFi + OTA module. Copy it into any ESP32 sketch folder to get OTA support with three lines of integration:

```cpp
#define OTA_LED_PIN 2   // optional
#include "ota_core.h"

void setup() { otaCoreSetup(); }
void loop()  { otaCoreHandle(); if (!otaCoreReady()) return; /* sketch logic */ }
```

Config via `#define` before the include (all optional):

| Define | Default | Effect |
|---|---|---|
| `OTA_LED_PIN` | _(none)_ | GPIO for status LED blink |
| `OTA_WIFI_TIMEOUT_MS` | `15000` | WiFi connect timeout |
| `OTA_RECONNECT_MS` | `5000` | Reconnect retry interval |

Credentials (`WIFI_SSID`, `WIFI_PASSWORD`, `OTA_HOSTNAME`, optional `OTA_PASSWORD`) come from `secrets.h`.

## Project Structure

This is a monorepo. Each subfolder under a board-family directory is one self-contained project. Projects must not share or cross-reference files with sibling folders.

```
arduino-projects/
  Mega_2560/
    4digits/
      sonic-mm-display/       # HC-SR04 -> 7-seg display, mm readout
      sonic-mm-detect/        # HC-SR04 detection with LED timeout (tested.ok)
      sonic-mm-detect-light/  # E18-D80NK IR sensor variant (wip.notok)
      counter-00-99/          # Button counter on 4-digit display (tested.ok)
      counter-0-9/            # Single-digit counter
  ESP32_dev/
    otacore/                  # OTA module source + demo sketch
      ota_core.h              # header-only OTA+WiFi — copy into new sketches
      otacore.ino             # reference/demo sketch
    otabasic/                 # OTA-enabled bringup sketch (tested.ok)
    sketch_mar20a/            # Basic ESP32 hello-world sketch
  docs/
    wiring-notation.md        # ATN-IO v3 format spec
```

Status markers: `tested.ok` = verified working on hardware; `wip.notok` = work in progress, may not function.

## Board-Specific Context

Determine the active board from the parent folder name (`Mega_2560`, `ESP32_dev`, etc.) and tailor all suggestions accordingly:

- **Mega_2560**: AVR ATmega2560, 5V logic, Arduino AVR core. Confirmed working pins per projects: shift register on D8/D9/D10, digit select D2–D5, sensor inputs on D6/D7, onboard LED D13.
- **ESP32_dev**: ESP32 WROOM, 3.3V logic. Do not assume AVR APIs are available.

If the board is ambiguous, ask for the target `.ino` path before making architecture-specific suggestions.

## Coding Standards

- Use `millis()`-based scheduling and state machines. Avoid `delay()` in normal control flow (`delayMicroseconds()` for tight hardware timing like TRIG pulses is acceptable).
- Minimize dynamic memory allocation; prefer `const`/`constexpr` for configuration values.
- Organize `.ino` files with sections separated by `// ── Section ──` banner comments.
- User-tunable parameters go at the top of the file under a `// ── User parameters` section.
- Keep ISR usage minimal.

## Wiring Documentation (ATN-IO v3)

Every project must have a `docs/wiring.md` file in **ATN-IO v3** format (spec: [docs/wiring-notation.md](docs/wiring-notation.md)).

Required structure (in order):
```
ATN-IO v3
Project: <short description>

[BOARD]      -- required: TYPE, LOGIC
[INPUTS]     -- optional: SIGNAL_NAME -> PIN
[OUTPUTS]    -- optional: SIGNAL_NAME -> PIN
[COMPONENTS] -- optional but recommended: REF = DESCRIPTION
[WIRING]     -- required: full electrical paths using -> chains
[POWER]      -- optional: non-standard rails
[NOTES]      -- optional: free-form
```

Key rules:
- `->` means direct electrical connection in series.
- `[INPUTS]`/`[OUTPUTS]` are logical mappings only — no resistors or power in these sections.
- Every current path in `[WIRING]` must terminate at `GND` or a named power rail.
- Every REF used in `[WIRING]` must appear in `[COMPONENTS]`.
- IC enable/OE/reset pins must be explicitly wired, never left floating.
- Use `.PIN<n>` for ICs; named terminals (`.A`, `.K`, `.VCC`, `.GND`) for discrete components.

Before reading or modifying a project's code, consult its `docs/wiring.md` for pin assignments and electrical context.
