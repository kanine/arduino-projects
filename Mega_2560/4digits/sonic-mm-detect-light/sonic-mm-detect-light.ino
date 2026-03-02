// Ultrasonic distance meter – HC-SR04 -> 4-digit 7-seg (5641AS) via 74HC595
// Displays distance in mm (0000–9999).
// Out-of-range: display shows "----", onboard LED off.
// In-range:     display shows mm reading, onboard LED on for SENSOR_TIMEOUT ms,
//               then off until a fresh in-range reading arrives.
//
// Mega pins used:
//   Shift register : DATA=D8, LATCH=D9, CLK=D10
//   Digit select   : DIG1=D2, DIG2=D3, DIG3=D4, DIG4=D5
//   HC-SR04        : TRIG=D6, ECHO=D7
//   Onboard LED    : D13 (built-in on all Arduino boards)

// ── User parameters ───────────────────────────────────────────────────────────
const unsigned int  OUT_OF_RANGE   = 500;   // mm – readings >= this = no target
const unsigned long SENSOR_TIMEOUT = 3000;   // ms – how long LED stays on after a hit

// ── Pin assignments ───────────────────────────────────────────────────────────
const int latch      = 9;
const int clockPin   = 10;
const int data       = 8;

const int DIG1 = 2;
const int DIG2 = 3;
const int DIG3 = 4;
const int DIG4 = 5;

const int TRIG        = 6;
const int ECHO        = 7;
const int ONBOARD_LED = 13;

// ── Timing ────────────────────────────────────────────────────────────────────
const unsigned long MEASURE_INTERVAL = 100;    // ms between sensor reads
const unsigned long ECHO_TIMEOUT_US  = 30000UL;

// ── Segment table: bit0=A … bit6=G, bit7=DP ──────────────────────────────────
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
const unsigned char SEG_DASH  = 0x40;  // segment G only  →  -
const unsigned char SEG_BLANK = 0x00;  // all segments off

// ── State ─────────────────────────────────────────────────────────────────────
unsigned int  displayValue = 0;
bool          showDash     = true;   // true = display "----"
bool          ledActive    = false;
unsigned long ledOnAt      = 0;

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

void showSegByte(int digPin, byte segByte) {
  shiftSeg(segByte);
  allDigitsOff();
  digitalWrite(digPin, LOW);
  delayMicroseconds(4000);
}

void showDigit(int digPin, byte value) {
  showSegByte(digPin, table[value]);
}

// ── Display multiplex ─────────────────────────────────────────────────────────
void refreshDisplay() {
  if (showDash) {
    showSegByte(DIG1, SEG_DASH);
    showSegByte(DIG2, SEG_DASH);
    showSegByte(DIG3, SEG_DASH);
    showSegByte(DIG4, SEG_DASH);
  } else {
    unsigned int val = displayValue;
    showDigit(DIG1, val / 1000);
    showDigit(DIG2, (val / 100) % 10);
    showDigit(DIG3, (val / 10)  % 10);
    showDigit(DIG4, val % 10);
  }
}

// ── Ultrasonic read ───────────────────────────────────────────────────────────
unsigned int readDistanceMM() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  unsigned long duration = pulseIn(ECHO, HIGH, ECHO_TIMEOUT_US);
  if (duration == 0) return 0xFFFF;

  unsigned long mm = (duration * 343UL) / 2000UL;
  if (mm > 9999) mm = 9999;
  return (unsigned int)mm;
}

// ── Setup ─────────────────────────────────────────────────────────────────────
void setup() {
  pinMode(latch,       OUTPUT);
  pinMode(clockPin,    OUTPUT);
  pinMode(data,        OUTPUT);
  pinMode(DIG1,        OUTPUT);
  pinMode(DIG2,        OUTPUT);
  pinMode(DIG3,        OUTPUT);
  pinMode(DIG4,        OUTPUT);
  pinMode(TRIG,        OUTPUT);
  pinMode(ECHO,        INPUT);
  pinMode(ONBOARD_LED, OUTPUT);

  digitalWrite(TRIG,        LOW);
  digitalWrite(ONBOARD_LED, LOW);
  allDigitsOff();

  Serial.begin(19200);
  Serial.print("OUT_OF_RANGE:   "); Serial.print(OUT_OF_RANGE);   Serial.println(" mm");
  Serial.print("SENSOR_TIMEOUT: "); Serial.print(SENSOR_TIMEOUT); Serial.println(" ms");
}

// ── Main loop ─────────────────────────────────────────────────────────────────
void loop() {
  static unsigned long lastMeasure = 0;
  unsigned long now = millis();

  // ── 1. Sensor read ────────────────────────────────────────────────────────
  if (now - lastMeasure >= MEASURE_INTERVAL) {
    lastMeasure = now;

    unsigned int mm  = readDistanceMM();
    bool inRange     = (mm != 0xFFFF && mm < OUT_OF_RANGE);

    if (inRange) {
      displayValue = mm;
      showDash     = false;

      // (Re)start the LED timer on every fresh in-range reading
      ledActive = true;
      ledOnAt   = now;
      digitalWrite(ONBOARD_LED, HIGH);

      Serial.print("Distance mm: ");
      Serial.println(mm);
    } else {
      showDash = true;
      Serial.println("Out of range");
    }
  }

  // ── 2. LED timeout check ──────────────────────────────────────────────────
  if (ledActive && (now - ledOnAt >= SENSOR_TIMEOUT)) {
    ledActive = false;
    digitalWrite(ONBOARD_LED, LOW);
  }

  // ── 3. Multiplex display ──────────────────────────────────────────────────
  refreshDisplay();
}
