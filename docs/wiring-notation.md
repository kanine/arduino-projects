# instructions.md  
## ATN-IO v3 – Arduino Text Notation

---

## Purpose

ATN-IO is a human-readable and machine-parseable text format for documenting Arduino and embedded wiring projects.

It separates:

- **Logical mapping** (what signal goes to which pin)
- **Physical wiring** (actual electrical connection path)
- **Components** (resistors, LEDs, sensors, modules)
- **Power rails**

It is designed to be:

- Easy to read
- Easy to write
- Git-friendly
- Deterministic to parse
- Suitable for code generation

---

## Core Principles

1. Logical mapping and physical wiring are separate.
2. One instruction per line.
3. Arrows `->` indicate a direct electrical connection.
4. Comments begin with `#`.
5. Sections are uppercase and appear once.
6. Be explicit about ground and power.

---

## File Structure

Sections must appear in this order:
[BOARD]
[INPUTS] (optional)
[OUTPUTS] (optional)
[COMPONENTS] (optional but recommended)
[WIRING] (mandatory)
[POWER] (optional)
[NOTES] (optional)


Only `[BOARD]` and `[WIRING]` are required.

---

## Section Definitions

---

### [BOARD]

Declares controller metadata.

Format:
KEY = VALUE


Example:

[BOARD]
TYPE = Arduino Mega 2560
LOGIC = 5V


---

### [INPUTS]

Maps logical input names to board pins.

Format:

NAME -> PIN

Example:


[INPUTS]
IR1 -> D2
SONIC1 -> D3


This is logical mapping only.  
It does NOT describe the full electrical path.

---

### [OUTPUTS]

Maps logical output names to board pins.

Format:

NAME -> PIN


Example:


[OUTPUTS]
LED1 -> D13
CUT_RELAY -> D8


### [COMPONENTS]

Lists physical parts used in the build.

Format:


REF = DESCRIPTION


Examples:


[COMPONENTS]
R1 = 220R
LED1 = LED, RED
RELAY1 = 5V Relay Module
IR1 = NPN_SENSOR, 24V


This section supports:
- Documentation
- BOM generation
- Validation

---

### [WIRING] (Mandatory)

Defines the actual electrical path.

Format:


NODE -> NODE -> NODE


Each arrow indicates a physical series connection.

Examples:

Simple LED:


[WIRING]
D13 -> R1 -> LED1.A
LED1.K -> GND


24V sensor:


[WIRING]
24V -> IR1.BROWN
IR1.BLACK -> D2
IR1.BLUE -> GND


Valid nodes:

- Board pins (`D13`, `A0`)
- Component references (`R1`)
- Component terminals (`LED1.A`, `LED1.K`)
- Power rails (`GND`, `5V`, `24V`)

Every current path must terminate logically (usually at GND or a rail).

---

### [POWER]

Declares named voltage rails.

Format:


NAME -> VOLTAGE


Example:


[POWER]
SENSORS_V+ -> 24V
COMMON_GND -> GND


---

### [NOTES]

Free-form human documentation.  
Ignored by parsers.

---

## Complete Example – Arduino Mega Blink

ATN-IO v3
Project: Arduino Mega Blink

[BOARD]
TYPE = Arduino Mega 2560
LOGIC = 5V

[OUTPUTS]
LED1 -> D13

[COMPONENTS]
R1 = 220R
LED1 = LED, RED

[WIRING]
D13 -> R1 -> LED1.A
LED1.K -> GND


---

## Design Rules

- Keep logical mapping clean and minimal.
- Always show full electrical path in `[WIRING]`.
- Do not mix physical chains inside `[INPUTS]` or `[OUTPUTS]`.
- Be explicit with polarity (A/K, +/−, NO/NC).
- Use consistent naming (LED1, R1, IR1, etc.).

---

## Why This Format Works

It mirrors industrial practice:

Logical mapping → Control intent  
Wiring section → Electrical reality  

This makes it:

- Clear for humans
- Structured for automation
- Safe for expanding to larger systems

---

## Recommended Naming Conventions

- Resistors: `R1`, `R2`
- LEDs: `LED1`
- Relays: `RELAY1`
- Sensors: `IR1`, `SONIC1`
- Power nets: `5V`, `24V`, `GND`

Consistency matters.

---

## Summary

ATN-IO v3 is:

Readable  
Deterministic  
Scalable  
Industrial-friendly  
Suitable for AI parsing  

It cleanly separates logic from wiring and keeps everything explicit.