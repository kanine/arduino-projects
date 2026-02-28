[PROJECT]
4-digit 7-seg (5641AS) + 74HC595 + Arduino Mega 2560
- Digit select is ACTIVE LOW (digit turns ON when its DIG pin is grounded)
- Segments are driven by 74HC595 through 220Ω resistors
- Display is multiplexed in software (one digit at a time, fast refresh)

[POWER]
MEGA_5V  -> BREADBOARD_+RAIL
MEGA_GND -> BREADBOARD_-RAIL

BREADBOARD_+RAIL (TOP) -> BREADBOARD_+RAIL (BOTTOM)   (if rails split)
BREADBOARD_-RAIL (TOP) -> BREADBOARD_-RAIL (BOTTOM)   (if rails split)

[IC: 74HC595 POWER/CONTROL]
74HC595_PIN16 (VCC)    -> +5V
74HC595_PIN8  (GND)    -> GND
74HC595_PIN10 (MR/SRCLR)-> +5V     (must be HIGH, else chip held in reset)
74HC595_PIN13 (OE)     -> GND      (must be LOW, else outputs disabled)

[IC: 74HC595 <-> ARDUINO CONTROL LINES]
MEGA_D8   -> 74HC595_PIN14 (DS / DATA)
MEGA_D10  -> 74HC595_PIN11 (SHCP / CLOCK)
MEGA_D9   -> 74HC595_PIN12 (STCP / LATCH)

[DISPLAY: 5641AS PINOUT (FROM YOUR DIAGRAM)]
TOP ROW PINS:  12=D1, 11=A, 10=F, 9=D2, 8=D3, 7=B
BOTTOM ROW:     1=E,  2=D,  3=DP, 4=C, 5=G, 6=D4

[DIGIT SELECT (ACTIVE LOW)]
MEGA_D2 -> DISPLAY_PIN12 (D1 / leftmost digit)
MEGA_D3 -> DISPLAY_PIN9  (D2)
MEGA_D4 -> DISPLAY_PIN8  (D3)
MEGA_D5 -> DISPLAY_PIN6  (D4 / rightmost digit)

[SEGMENTS: 74HC595 OUTPUTS -> (220Ω) -> DISPLAY SEGMENT PINS]
# NOTE: each line must have ONE 220Ω resistor in series.

74HC595_PIN15 (Q0) -> 220R -> DISPLAY_PIN11 (A)
74HC595_PIN1  (Q1) -> 220R -> DISPLAY_PIN7  (B)
74HC595_PIN2  (Q2) -> 220R -> DISPLAY_PIN4  (C)
74HC595_PIN3  (Q3) -> 220R -> DISPLAY_PIN2  (D)
74HC595_PIN4  (Q4) -> 220R -> DISPLAY_PIN1  (E)
74HC595_PIN5  (Q5) -> 220R -> DISPLAY_PIN10 (F)
74HC595_PIN6  (Q6) -> 220R -> DISPLAY_PIN5  (G)
74HC595_PIN7  (Q7) -> 220R -> DISPLAY_PIN3  (DP)

[NOTES / CHECKS]
- If nothing lights: verify 74HC595 OE(pin13)=GND and MR(pin10)=+5V
- If random segments: verify LATCH is MEGA_D9 -> 74HC595 pin12
- If a specific segment never lights: check the resistor + breadboard row alignment for that segment line