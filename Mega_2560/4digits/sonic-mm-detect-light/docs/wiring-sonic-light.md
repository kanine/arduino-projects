# Ultrasonic Distance Meter Wiring (Arduino Mega 2560)

This document describes the wiring for an **HC-SR04 ultrasonic sensor** and a **4-digit 7-segment display (5641AS, common cathode)** driven via a **74HC595** shift register.

## Board

- **Board:** Arduino Mega 2560
- **Logic level:** 5V

## Pin Assignment Summary

### Inputs

| Signal | Mega Pin |
|---|---|
| ECHO | D7 |

### Outputs

| Signal | Mega Pin |
|---|---|
| TRIG | D6 |
| DATA | D8 |
| LATCH | D9 |
| CLK | D10 |
| DIG1 | D2 |
| DIG2 | D3 |
| DIG3 | D4 |
| DIG4 | D5 |

## Components

| Ref | Value / Part | Notes |
|---|---|---|
| U1 | 74HC595 | Shift register |
| DSP | 5641AS 4-digit 7-segment | Common cathode |
| SONIC1 | HC-SR04 | Ultrasonic distance sensor |
| R1-R8 | 220R each | Segment current limiting (A, B, C, D, E, F, G, DP) |

## Wiring

### 1) Power

| From | To | Purpose |
|---|---|---|
| 5V | U1.PIN16 | 74HC595 VCC |
| GND | U1.PIN8 | 74HC595 GND |
| 5V | U1.PIN10 | MR/SRCLR held HIGH (no reset) |
| GND | U1.PIN13 | OE held LOW (outputs enabled) |

### 2) Arduino -> 74HC595 Control Lines

| Mega Pin | 74HC595 Pin | Label |
|---|---|---|
| D8 | U1.PIN14 | DS / DATA |
| D10 | U1.PIN11 | SHCP / CLOCK |
| D9 | U1.PIN12 | STCP / LATCH |

### 3) 74HC595 Outputs -> Resistors -> Display Segments

| 74HC595 | Resistor | Display Pin | Segment |
|---|---|---|---|
| U1.PIN15 (Q0) | R1 | DSP.PIN11 | A |
| U1.PIN1 (Q1) | R2 | DSP.PIN7 | B |
| U1.PIN2 (Q2) | R3 | DSP.PIN4 | C |
| U1.PIN3 (Q3) | R4 | DSP.PIN2 | D |
| U1.PIN4 (Q4) | R5 | DSP.PIN1 | E |
| U1.PIN5 (Q5) | R6 | DSP.PIN10 | F |
| U1.PIN6 (Q6) | R7 | DSP.PIN5 | G |
| U1.PIN7 (Q7) | R8 | DSP.PIN3 | DP |

### 4) Digit Select (Active LOW)

| Mega Pin | Display Pin | Digit |
|---|---|---|
| D2 | DSP.PIN12 | DIG1 (leftmost) |
| D3 | DSP.PIN9 | DIG2 |
| D4 | DSP.PIN8 | DIG3 |
| D5 | DSP.PIN6 | DIG4 (rightmost) |

### 5) HC-SR04

| From | To |
|---|---|
| 5V | SONIC1.VCC |
| GND | SONIC1.GND |
| D6 | SONIC1.TRIG |
| SONIC1.ECHO | D7 |

## Notes

- Segment bit order in the shift-register byte: `bit0=A`, `bit1=B`, `...`, `bit6=G`, `bit7=DP`.
- The display is common-cathode: digit pin `LOW = ON`, `HIGH = OFF`.
- HC-SR04 operates at 5V and is directly compatible with Mega 2560 logic levels.
- TRIG pulse is `10 us` HIGH; ECHO pulse width is proportional to distance.
- Distance formula: `mm = (echo_us * 343) / 2000`.
- Readings are taken every `100 ms`; display is continuously multiplexed between readings.
- Display range is `0000-9999 mm`; `9999` is shown if out of range or no echo is received.
- HC-SR04 shares the breadboard +5V and GND rails (no additional rail wiring needed).
