# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Building and Uploading

There is no CLI build system. Sketches are compiled and uploaded via **Arduino IDE 2**. Open the `.ino` file directly in Arduino IDE 2, select the correct board and port, then use Sketch > Upload.

For command-line workflows, `arduino-cli` can be used if installed:
```bash
arduino-cli compile --fqbn arduino:avr:mega path/to/sketch/
arduino-cli upload  --fqbn arduino:avr:mega -p /dev/ttyUSB0 path/to/sketch/
```

Serial monitor baud rates in use: **19200** (Mega 2560 projects), **115200** (ESP32 projects).

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
