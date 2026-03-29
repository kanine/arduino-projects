ATN-IO v3
Project: timeofflightwebhook2 — ESP32 VL53L1X webhook client with runtime config updates

[BOARD]
TYPE  = ESP32 WROOM dev board
LOGIC = 3.3V

[INPUTS]
SDA -> D21
SCL -> D22

[OUTPUTS]
LED -> GPIO2
SPEAKER -> D4

[COMPONENTS]
U1 = VL53L1X time-of-flight sensor breakout (I2C addr 0x29)
LED1 = Onboard or external status LED
BZR1 = Active buzzer / speaker module, 3.3V-compatible input

[WIRING]
3V3 -> U1.VIN
GND -> U1.GND

D21 -> U1.SDA
D22 -> U1.SCL

GPIO2 -> LED1.A
LED1.K -> GND

D4 -> BZR1.SIG
3V3 -> BZR1.VCC
GND -> BZR1.GND

; Optional sensor side pins
; XSHUT may be left unconnected for single-sensor use
; GPIO1/IRQ not used by this sketch

[POWER]
3V3 = ESP32 3.3V rail

[NOTES]
- Logic level is 3.3V on the ESP32.
- Default firmware pins are SDA=21, SCL=22, LED=2, SPEAKER=4; change them in config.h if needed.
- Install `VL53L1X (1.3.1)` by Pololu from Arduino Library Manager.
- Status LED feedback is on GPIO2. The speaker on D4 is used for failure alerts only.
