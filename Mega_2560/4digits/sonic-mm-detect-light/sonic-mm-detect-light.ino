// Object detector – E18-D80NK -> 4-digit 7-seg (5641AS) via 74HC595
// E18-D80NK output is open collector and active LOW when object is detected.
// Detected:    display shows "0000", onboard LED on for SENSOR_TIMEOUT ms.
// Not detected: display shows "----", onboard LED off (unless timeout window active).
//
// Mega pins used:
//   Shift register : DATA=D8, LATCH=D9, CLK=D10
//   Digit select   : DIG1=D2, DIG2=D3, DIG3=D4, DIG4=D5
//   E18-D80NK OUT  : D6 (with external 10k pull-up to +5V)
//   Onboard LED    : D13 (built-in on all Arduino boards)

// ── User parameters ───────────────────────────────────────────────────────────
const unsigned long SENSOR_TIMEOUT = 3000;   // ms – how long LED stays on after a hit

// ── Pin assignments ───────────────────────────────────────────────────────────
const int latch      = 9;
const int clockPin   = 10;
const int data       = 8;

const int DIG1 = 2;
const int DIG2 = 3;
const int DIG3 = 4;
const int DIG4 = 5;

const int SENSOR_OUT  = 6;   // E18-D80NK black wire
const int ONBOARD_LED = 13;

// ── Timing ────────────────────────────────────────────────────────────────────
const unsigned long MEASURE_INTERVAL = 20;     // ms between sensor reads
const bool          DEBUG_SENSOR_RAW = true;   // print raw D6 state to Serial
const unsigned long DEBUG_INTERVAL   = 200;    // ms between debug lines

// ── Sensor logic options ──────────────────────────────────────────────────────
const bool SENSOR_ACTIVE_LOW   = false;   // E18 default: LOW means detected
const bool USE_INTERNAL_PULLUP = true;   // experiment mode; keep external 10k if present

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

// ── E18-D80NK read ────────────────────────────────────────────────────────────
bool isObjectDetected() {
  int raw = digitalRead(SENSOR_OUT);
  return SENSOR_ACTIVE_LOW ? (raw == LOW) : (raw == HIGH);
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
  pinMode(SENSOR_OUT,  USE_INTERNAL_PULLUP ? INPUT_PULLUP : INPUT);
  pinMode(ONBOARD_LED, OUTPUT);

  digitalWrite(ONBOARD_LED, LOW);
  allDigitsOff();

  Serial.begin(19200);
  Serial.print("SENSOR_TIMEOUT: "); Serial.print(SENSOR_TIMEOUT); Serial.println(" ms");
  Serial.print("Sensor: E18-D80NK active ");
  Serial.println(SENSOR_ACTIVE_LOW ? "LOW" : "HIGH");
  Serial.print("Internal pull-up: ");
  Serial.println(USE_INTERNAL_PULLUP ? "ON" : "OFF");
}

// ── Main loop ─────────────────────────────────────────────────────────────────
void loop() {
  static unsigned long lastMeasure = 0;
  static unsigned long lastDebugMs  = 0;
  static bool lastDetected          = false;
  unsigned long now = millis();

  // ── 1. Sensor read ────────────────────────────────────────────────────────
  if (now - lastMeasure >= MEASURE_INTERVAL) {
    lastMeasure = now;

    bool detected = isObjectDetected();

    if (detected) {
      displayValue = 0;
      showDash     = false;

      // (Re)start the LED timer on every fresh detection
      ledActive = true;
      ledOnAt   = now;
      digitalWrite(ONBOARD_LED, HIGH);

      if (!lastDetected) {
        Serial.println("Object detected");
      }
    } else {
      showDash = true;

      if (lastDetected) {
        Serial.println("No object");
      }
    }

    lastDetected = detected;
  }

  // ── 2. Optional raw input debug ────────────────────────────────────────────
  if (DEBUG_SENSOR_RAW && (now - lastDebugMs >= DEBUG_INTERVAL)) {
    lastDebugMs = now;
    int raw = digitalRead(SENSOR_OUT);
    Serial.print("D6 raw=");
    Serial.print(raw == LOW ? "LOW" : "HIGH");
    Serial.print(" detected=");
    bool detected = SENSOR_ACTIVE_LOW ? (raw == LOW) : (raw == HIGH);
    Serial.println(detected ? "YES" : "NO");
  }

  // ── 3. LED timeout check ──────────────────────────────────────────────────
  if (ledActive && (now - ledOnAt >= SENSOR_TIMEOUT)) {
    ledActive = false;
    digitalWrite(ONBOARD_LED, LOW);
  }

  // ── 4. Multiplex display ──────────────────────────────────────────────────
  refreshDisplay();
}
