ATN-IO v3
Project: httpcore — header-only HTTP POST module; no external components

[BOARD]
TYPE  = ESP32 WROOM dev board
LOGIC = 3.3V

[INPUTS]

[OUTPUTS]
LED -> GPIO2 (onboard blue LED, active-HIGH, optional — blinks 3x on POST failure)

[COMPONENTS]

[WIRING]
; Onboard LED is internal to the dev board
GPIO2 -> onboard_LED.A -> onboard_LED.K -> GND   ; ~470 Ω series resistor built in to board

[NOTES]
- http_core.h is a software module only; all connectivity is via the existing WiFi radio
- HTTP_LED_PIN is optional — omit the define to disable error blink entirely
- Pair with ota_core.h so WiFi and OTA are both handled before calling httpPost()
