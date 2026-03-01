// Two-digit multiplex: steady "99" on DIG3 and DIG4
// 74HC595 shift register on pins 8/9/10
// DIG3 (pin 4) = tens, DIG4 (pin 5) = units
// Common-cathode display: digit pin LOW = ON

const int latch    = 9;
const int clockPin = 10;
const int data     = 8;

const int DIG1 = 2;   // unused here, keep forced OFF
const int DIG2 = 3;   // unused here, keep forced OFF
const int DIG3 = 4;   // tens
const int DIG4 = 5;   // units

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

byte count = 0;
unsigned long lastCount = 0;
constexpr unsigned long COUNT_INTERVAL_MS = 700;
// Per-digit on-time calibration (microseconds).
// 1 and 4 are intentionally lower because they appear much brighter on this setup.
const unsigned int DIGIT_ON_US[10] = {
  3400, // 0
  1700, // 1
  3100, // 2
  3100, // 3
  2200, // 4
  3200, // 5
  3400, // 6
  2500, // 7
  3600, // 8
  3400  // 9
};

void shiftSeg(byte s) {
  digitalWrite(latch, LOW);
  shiftOut(data, clockPin, MSBFIRST, s);
  digitalWrite(latch, HIGH);
}

void setup() {
  pinMode(latch,    OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(data,     OUTPUT);
  pinMode(DIG1,     OUTPUT);
  pinMode(DIG2,     OUTPUT);
  pinMode(DIG3,     OUTPUT);
  pinMode(DIG4,     OUTPUT);

  // All digits off (HIGH = off for common-cathode)
  digitalWrite(DIG1, HIGH);
  digitalWrite(DIG2, HIGH);
  digitalWrite(DIG3, HIGH);
  digitalWrite(DIG4, HIGH);
}

void loop() {
  byte tens = count / 10;
  byte units = count % 10;
  unsigned int tensOnUs = DIGIT_ON_US[tens];
  unsigned int unitsOnUs = DIGIT_ON_US[units];

  // Keep unused digits OFF
  digitalWrite(DIG1, HIGH);
  digitalWrite(DIG2, HIGH);

  // Phase A: show tens on DIG3 only
  shiftSeg(table[tens]);
  digitalWrite(DIG3, LOW);
  digitalWrite(DIG4, HIGH);
  delayMicroseconds(tensOnUs);

  // Phase B: show units on DIG4 only
  shiftSeg(table[units]);
  digitalWrite(DIG3, HIGH);
  digitalWrite(DIG4, LOW);
  delayMicroseconds(unitsOnUs);

  unsigned long now = millis();
  if (now - lastCount >= COUNT_INTERVAL_MS) {
    lastCount = now;
    count = (count + 1) % 100;
  }
}
