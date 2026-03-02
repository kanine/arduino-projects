# E18-D80NK Object Detector Wiring (Arduino Mega 2560)

This document describes the wiring for an **E18-D80NK infrared obstacle sensor** and a **4-digit 7-segment display (5641AS, common cathode)** driven via a **74HC595** shift register.

## Board

- **Board:** Arduino Mega 2560
- **Logic level:** 5V

## Pin Assignment Summary

### Inputs

| Signal | Mega Pin |
|---|---|
| E18_OUT | D6 |

### Outputs

| Signal | Mega Pin |
|---|---|
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
| SENSOR1 | E18-D80NK | Infrared reflective obstacle sensor |
| R1-R8 | 220R each | Segment current limiting (A, B, C, D, E, F, G, DP) |
| R9 | 10K | Pull-up for E18 output (open collector) |

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

### 5) E18-D80NK

| From | To |
|---|---|
| 5V | SENSOR1 brown wire |
| GND | SENSOR1 blue wire |
| SENSOR1 black wire | D6 |
| R9 (10K) | Between D6 and +5V |

## Notes

- Segment bit order in the shift-register byte: `bit0=A`, `bit1=B`, `...`, `bit6=G`, `bit7=DP`.
- The display is common-cathode: digit pin `LOW = ON`, `HIGH = OFF`.
- E18-D80NK operates at 5V and uses an open-collector NPN output.
- Output behavior: `LOW = object detected`, `HIGH = no object` (via pull-up resistor).
- Detection distance is adjustable on the sensor body (roughly `3 cm` to `80 cm`, target-dependent).
- The sketch samples the sensor every `20 ms`; display is continuously multiplexed between samples.
- Display behavior: `0000` when detected, `----` when no object is detected.
