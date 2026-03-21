ATN-IO v3
Project: ESP32 OTA basic – WiFi OTA update target

[BOARD]
  TYPE  = ESP32 WROOM-32
  LOGIC = 3.3V

[OUTPUTS]
  LED_STATUS -> GPIO2

[COMPONENTS]
  U1  = ESP32 WROOM-32 dev board
  D1  = Onboard blue LED (active HIGH, connected to GPIO2 via on-board resistor)

[WIRING]
  U1.GPIO2 -> D1.A -> D1.K -> GND

[NOTES]
  No external components. The onboard LED (D1) is driven directly by GPIO2.
  LED behaviour:
    2 blinks = OTA ready
    solid ON  = OTA transfer in progress
    3 blinks  = OTA complete
    5 blinks  = OTA error
  All other I/O pins are unused in this sketch.
