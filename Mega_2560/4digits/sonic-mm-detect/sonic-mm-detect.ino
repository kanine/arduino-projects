// Ultrasonic distance meter – HC-SR04 -> 4-digit 7-seg (5641AS) via 74HC595
// Displays distance in mm (0000 – 9999).
// Mega pins used:
//   Shift register : DATA=D8, LATCH=D9, CLK=D10   (unchanged)
//   Digit select   : DIG1=D2, DIG2=D3, DIG3=D4, DIG4=D5  (unchanged)
//   HC-SR04        : TRIG=D6, ECHO=D7              (new)
//
// Strategy: ultrasonic pulse is fired once per MEASURE_INTERVAL ms.
// pulseIn() is called with a short timeout so it never blocks the
// display multiplex loop for more than ~25 ms in the worst case.

// ── Pin assignments ──────────────────────────────────────────────────────────
const int latch    = 9;
const int clockPin = 10;
const int data     = 8;

const int DIG1 = 2;
const int DIG2 = 3;
const int DIG3 = 4;
const int DIG4 = 5;

const int TRIG = 6;
const int ECHO = 7;

// ── Timing ───────────────────────────────────────────────────────────────────
// How often to fire a new ultrasonic pulse (ms).
// HC-SR04 datasheet recommends ≥ 60 ms between pulses.
const unsigned long MEASURE_INTERVAL = 100;

// pulseIn timeout: 30 000 µs ≈ 515 cm max range (well past HC-SR04's ~400 cm).
// Keeps the block under ~30 ms worst-case.
const unsigned long ECHO_TIMEOUT_US = 30000UL;

// ── Segment lookup table (A..G + DP, bit0=A … bit6=G, bit7=DP) ──────────────
const unsigned char table[] = {
  0x3f, // 0
  0x06, // 1
  0x5b, // 2
  0x4f, // 3
  0x66, // 4
  0x6d, // 5
  0x7d, // 6
  0x07, // 7
  0x7f, // 8
  0x6f  // 9
};

// Shared state
volatile unsigned int displayValue = 0;   // 0–9999, updated by sensor read

// ── Shift register helpers ────────────────────────────────────────────────────
void shiftSeg(byte s) {
  digitalWrite(latch, LOW);
  shiftOut(data, clockPin, MSBFIRST, s);
  digitalWrite(latch, HIGH);
}

void allDigitsOff() {
  digitalWrite(DIG1, HIGH);
  digitalWrite(DIG2, HIGH);
  digitalWrite(DIG3, HIGH);
  digitalWrite(DIG4, HIGH);
}

void showDigit(int digPin, byte value) {
  shiftSeg(table[value]);
  allDigitsOff();
  digitalWrite(digPin, LOW);
  delayMicroseconds(4000);
}

// ── Ultrasonic measurement ────────────────────────────────────────────────────
// Returns distance in mm, clamped to 9999.
// Returns 9999 if echo times out (out of range).
unsigned int readDistanceMM() {
  // Send 10 µs trigger pulse
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  // Measure echo pulse width
  unsigned long duration = pulseIn(ECHO, HIGH, ECHO_TIMEOUT_US);

  if (duration == 0) {
    return 9999; // timeout / out of range
  }

  // Speed of sound ≈ 343 m/s = 0.343 mm/µs
  // distance = duration * 0.343 / 2  (round trip)
  // Multiply first to keep integer precision:
  //   duration(µs) * 343 / 2000  = mm
  unsigned long mm = (duration * 343UL) / 2000UL;

  if (mm > 9999) mm = 9999;
  return (unsigned int)mm;
}

// ── Setup ─────────────────────────────────────────────────────────────────────
void setup() {
  pinMode(latch,    OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(data,     OUTPUT);
  pinMode(DIG1,     OUTPUT);
  pinMode(DIG2,     OUTPUT);
  pinMode(DIG3,     OUTPUT);
  pinMode(DIG4,     OUTPUT);
  pinMode(TRIG,     OUTPUT);
  pinMode(ECHO,     INPUT);

  digitalWrite(TRIG, LOW);
  allDigitsOff();

  // Optional: Serial monitor for debugging
  Serial.begin(19200);
}

// ── Main loop ─────────────────────────────────────────────────────────────────
void loop() {
  static unsigned long lastMeasure = 0;

  // ── 1. Take a new ultrasonic reading on schedule ──────────────────────────
  unsigned long now = millis();
  if (now - lastMeasure >= MEASURE_INTERVAL) {
    lastMeasure = now;
    displayValue = readDistanceMM();
    Serial.print("Distance mm: ");
    Serial.println(displayValue);
  }

  // ── 2. Multiplex the display every loop iteration ─────────────────────────
  unsigned int val = displayValue;   // local copy; safe – single word read on AVR
  byte d1 = val / 1000;
  byte d2 = (val / 100) % 10;
  byte d3 = (val / 10)  % 10;
  byte d4 = val % 10;

  showDigit(DIG1, d1);
  showDigit(DIG2, d2);
  showDigit(DIG3, d3);
  showDigit(DIG4, d4);
}
