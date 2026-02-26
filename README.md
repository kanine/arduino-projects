# Arduino Projects Monorepo

This repository is a monorepo of self-contained Arduino projects intended for Arduino IDE 2.

## Project Structure
- Each sub-folder is one complete project.
- Each project should contain its own `.ino` entry sketch and any local assets needed by that project.
- Projects should not depend on files from sibling folders unless explicitly planned.

Example layout:

```text
arduino-projects/
  Uno/
    Uno.ino
  ESP32/
    ESP32.ino
  Nano_BLE/
    Nano_BLE.ino
```

## Folder Naming Convention
- Use the board family as the folder name.
- Prefer consistent `Pascal_Case` for multi-word names.
- Keep names short and architecture-specific.

## Popular Boards and Recommended Folder Names

| Board / Family | Recommended Folder Name |
|---|---|
| Arduino Uno (ATmega328P) | `Uno` |
| Arduino Mega 2560 | `Mega_2560` |
| Arduino Nano (classic) | `Nano` |
| Arduino Nano Every | `Nano_Every` |
| Arduino Nano 33 BLE / BLE Sense | `Nano_BLE` |
| ESP32 Dev Module / WROOM / S3 variants | `ESP32` |
| ESP8266 (NodeMCU, D1 mini) | `ESP8266` |
| Raspberry Pi Pico / Pico W (RP2040) | `RP2040` |
| Arduino Leonardo | `Leonardo` |
| Arduino Due | `Due` |

## Development Standards (Arduino IDE 2)
- Use modern C++ supported by Arduino IDE 2 toolchains.
- Prefer non-blocking code patterns (`millis()` scheduling, state machines).
- Avoid `delay()` in normal control flow.
- Install and manage dependencies through Arduino IDE 2 Library Manager when possible.
