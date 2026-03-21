ATN-IO v3
Project: httpbasic — http_core.h demo; posts JSON heartbeat to webhook every 10 s

[BOARD]
TYPE  = ESP32 WROOM dev board
LOGIC = 3.3V

[INPUTS]

[OUTPUTS]
LED -> GPIO2 (onboard blue LED, active-HIGH)

[COMPONENTS]

[WIRING]
; Onboard LED is internal to the dev board
GPIO2 -> onboard_LED.A -> onboard_LED.K -> GND   ; ~470 Ω series resistor built in to board

[NOTES]
- No external components needed; all I/O is via WiFi
- LED blinks 3x on POST failure or missing success:true in webhook response
- WEBHOOK_URL defined in secrets.h
