ATN-IO v3
Project: touch-sensor — capacitive touch test; onboard LED lights on finger contact

[BOARD]
TYPE  = ESP32 WROOM dev board
LOGIC = 3.3V

[INPUTS]
TOUCH -> T0 (GPIO4)

[OUTPUTS]
LED -> GPIO2 (onboard blue LED, active-HIGH)

[COMPONENTS]
PAD = ~10 cm jumper wire used as touch pad (finger antenna)

[WIRING]
; Capacitive touch input — no DC path; ESP32 internal touch circuit measures
; the change in capacitance on the pin when a finger approaches.
GPIO4 -> PAD   ; attach jumper wire here; touch the free end with a finger

; Onboard LED is internal to the dev board
GPIO2 -> onboard_LED.A -> onboard_LED.K -> GND   ; ~470 Ω series resistor built in to board

[NOTES]
- touchRead(T0) returns ~60–80 when untouched, ~5–30 when finger contacts PAD
- TOUCH_THRESHOLD defaults to 40; increase if misses touches, decrease if false triggers
- GPIO4 must not be driven by external circuitry — capacitive input only
- During OTA flash, the LED will blink for OTA status callbacks; this is expected
