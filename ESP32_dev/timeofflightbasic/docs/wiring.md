ATN-IO v3
Project: timeofflightbasic — VL53L1X ToF sensor; posts distance_mm to webhook every 5 s

[BOARD]
TYPE  = ESP32 WROOM dev board
LOGIC = 3.3V

[INPUTS]
SDA -> D21
SCL -> D22

[OUTPUTS]
LED  -> GPIO2 (onboard blue LED, active-HIGH)
BUZZ -> GPIO4

[COMPONENTS]
U1 = Adafruit VL53L1X breakout (ToF distance sensor, I2C addr 0x29)
B1 = DFRobot DFR0032 active buzzer (Gravity 3-pin, onboard driver transistor)

[WIRING]
; Power
3V3 -> U1.VIN
GND -> U1.GND

; I2C bus (breakout has on-board 2.2 kΩ pull-ups on SDA and SCL)
D21 -> U1.SDA
D22 -> U1.SCL

; XSHUT — breakout pull-up holds HIGH (normal operation); leave unconnected
; GPIO1/IRQ — not used; leave unconnected

; Onboard LED (internal to dev board)
GPIO2 -> onboard_LED.A -> onboard_LED.K -> GND   ; ~470 Ω series resistor built in to board

; Buzzer (DFR0032 — Gravity 3-pin)
3V3   -> B1.VCC
GND   -> B1.GND
GPIO4 -> B1.SIG

[NOTES]
- Sensor runs in short distance mode (max ~1.3 m), 50 ms timing budget
- I2C default pins: SDA=GPIO21, SCL=GPIO22
- XSHUT and IRQ pins on breakout are unused; breakout pull-ups keep them in safe states
- WEBHOOK_URL defined in secrets.h
- Buzzer: double beep on WiFi connect, single beep every 3 s while WiFi is failing; non-blocking state machine (50 ms beep / 125 ms gap)
