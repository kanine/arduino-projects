# DFR0032 — Digital Buzzer Module

**Manufacturer:** DFRobot  
**SKU:** DFR0032  
**Series:** Gravity (digital)  
**Source:** [Core Electronics](https://core-electronics.com.au/digital-buzzer-module.html) / [DFRobot Wiki](https://wiki.dfrobot.com/Digital_Buzzer_Module__SKU__DFR0032_)

---

## Overview

A simple active buzzer on a Gravity-compatible breakout board. Drive it HIGH or LOW from any digital GPIO pin to produce a tone. The onboard transistor means it can be driven directly from a 3.3 V logic pin (e.g. Raspberry Pi, ESP32) even though the nominal supply voltage is 5 V.

---

## Specifications

| Parameter        | Value              |
|------------------|--------------------|
| Interface type   | Digital            |
| Supply voltage   | 3.3 V – 5 V DC     |
| Logic level      | 3.3 V / 5 V        |
| Connector        | Gravity 3-pin JST  |
| Mounting holes   | 2 × M3, 5 cm pitch |

---

## Connector Pinout

The board uses a **Gravity 3-pin JST connector**. There are no silkscreen labels on the PCB itself.  
Pin order is standard across the entire Gravity digital sensor range.

```
Connector face-on (looking at the plug end of the cable with flat side facing up)
┌───┬───┬───┐
│ 1 │ 2 │ 3 │
└───┴───┴───┘
  G   V   S
```

| Pin | Label  | Wire Colour (stock Gravity cable) | Description          |
|-----|--------|-----------------------------------|----------------------|
| 1   | GND    | **Black**                         | Ground               |
| 2   | VCC    | **Red**                           | 3.3 V or 5 V supply  |
| 3   | SIG    | **Yellow**                        | Digital signal input |

> **Note:** Pin 1 is closest to the **edge of the PCB**; pin 3 is closest to the centre. When the connector is oriented with the latch/tab facing up, the black wire is on the left.

---

## Wiring — 3.3 V Host (e.g. Raspberry Pi, ESP32, Arduino UNO Q)

| Buzzer Pin | Host Pin          |
|------------|-------------------|
| GND        | GND               |
| VCC        | 3.3 V             |
| SIG        | Any digital GPIO  |

The module works at 3.3 V despite the datasheet listing 5 V as nominal — the onboard driver transistor will fire reliably at 3.3 V logic.

---

## Basic Arduino / UNO Q Sample Code

```cpp
const int BUZZ_PIN = 3;  // connect SIG to digital pin 3

void setup() {
  pinMode(BUZZ_PIN, OUTPUT);
}

void loop() {
  // ~500 Hz tone
  digitalWrite(BUZZ_PIN, HIGH);
  delay(1);
  digitalWrite(BUZZ_PIN, LOW);
  delay(1);
}
```

To change pitch, adjust the `delay()` values — shorter delays = higher frequency.  
To silence the buzzer, hold the pin LOW.

---

## Physical Notes

- Board dimensions: ~30 mm × 22 mm
- Two M3 mounting holes, 5 cm (50 mm) centre-to-centre — standard Gravity grid spacing
- The large black dome on the PCB is the buzzer element itself
- The small IC beside it is the driver transistor

---

## Links

- [DFRobot Wiki (DFR0032)](https://wiki.dfrobot.com/Digital_Buzzer_Module__SKU__DFR0032_)
- [Schematic (PDF)](https://core-electronics.com.au/attachments/localcontent/20170719173339yekhk0_940540cc846.pdf)
- [Core Electronics product page](https://core-electronics.com.au/digital-buzzer-module.html)