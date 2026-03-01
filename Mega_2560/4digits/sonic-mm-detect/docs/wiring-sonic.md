ATN-IO v3
Project: Ultrasonic Distance Meter – HC-SR04 + 4-digit 7-seg (5641AS) + 74HC595 + Arduino Mega 2560

[BOARD]
TYPE  = Arduino Mega 2560
LOGIC = 5V

[INPUTS]
ECHO -> D7

[OUTPUTS]
TRIG -> D6
DATA -> D8
LATCH -> D9
CLK  -> D10
DIG1 -> D2
DIG2 -> D3
DIG3 -> D4
DIG4 -> D5

[COMPONENTS]
R1  = 220R   # segment A
R2  = 220R   # segment B
R3  = 220R   # segment C
R4  = 220R   # segment D
R5  = 220R   # segment E
R6  = 220R   # segment F
R7  = 220R   # segment G
R8  = 220R   # segment DP
U1  = 74HC595 shift register
DSP = 5641AS 4-digit 7-segment display (common cathode)
SONIC1 = HC-SR04 ultrasonic distance sensor

[WIRING]
# ── Power ─────────────────────────────────────────────────────────────────────
5V  -> U1.PIN16           # VCC
GND -> U1.PIN8            # GND
5V  -> U1.PIN10           # MR/SRCLR – must be HIGH (prevents reset)
GND -> U1.PIN13           # OE       – must be LOW  (enables outputs)

# ── Arduino -> 74HC595 control lines ─────────────────────────────────────────
D8  -> U1.PIN14           # DS   / DATA
D10 -> U1.PIN11           # SHCP / CLOCK
D9  -> U1.PIN12           # STCP / LATCH

# ── 74HC595 outputs -> resistors -> display segments ─────────────────────────
U1.PIN15 -> R1 -> DSP.PIN11   # Q0 -> A
U1.PIN1  -> R2 -> DSP.PIN7    # Q1 -> B
U1.PIN2  -> R3 -> DSP.PIN4    # Q2 -> C
U1.PIN3  -> R4 -> DSP.PIN2    # Q3 -> D
U1.PIN4  -> R5 -> DSP.PIN1    # Q4 -> E
U1.PIN5  -> R6 -> DSP.PIN10   # Q5 -> F
U1.PIN6  -> R7 -> DSP.PIN5    # Q6 -> G
U1.PIN7  -> R8 -> DSP.PIN3    # Q7 -> DP

# ── Digit select (active LOW) ─────────────────────────────────────────────────
D2 -> DSP.PIN12           # DIG1 – leftmost
D3 -> DSP.PIN9            # DIG2
D4 -> DSP.PIN8            # DIG3
D5 -> DSP.PIN6            # DIG4 – rightmost

# ── HC-SR04 ──────────────────────────────────────────────────────────────────
5V  -> SONIC1.VCC
GND -> SONIC1.GND
D6  -> SONIC1.TRIG
SONIC1.ECHO -> D7

[POWER]
5V  -> 5V
GND -> GND

[NOTES]
# Segment bit order in shift register byte: bit0=A, bit1=B, ..., bit6=G, bit7=DP
# Display is common-cathode: digit pin LOW = digit ON, HIGH = digit OFF
# HC-SR04 operating voltage: 5V, compatible with Mega logic levels
# TRIG pulse: 10 µs HIGH; ECHO pulse width proportional to distance
# Distance formula: mm = (echo_us * 343) / 2000
# Readings are taken every 100 ms; display is continuously multiplexed between readings
# Displays 0000–9999 mm; shows 9999 if target is out of range or no echo received
# HC-SR04 shares the breadboard +5V and GND rails – no additional wiring needed
