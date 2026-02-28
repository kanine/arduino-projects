// 2-digit multiplexed counter: 00 to 99
// 74HC595 shift register on pins 8/9/10
// DIG3 (pin 4) = tens digit, DIG4 (pin 5) = units digit
// Common-cathode display: digit pin LOW = ON

const int latch    = 9;
const int clockPin = 10;
const int data     = 8;

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

byte count       = 0;
byte muxDigit    = 0;   // 0 = tens, 1 = units

unsigned long lastMux   = 0;
unsigned long lastCount = 0;

constexpr unsigned long MUX_INTERVAL   = 5;    // ms per digit (≈100 Hz refresh)
constexpr unsigned long COUNT_INTERVAL = 700;  // ms between count steps

void shiftSeg(byte s) {
  digitalWrite(latch, LOW);
  shiftOut(data, clockPin, MSBFIRST, s);
  digitalWrite(latch, HIGH);
}

void setup() {
  pinMode(latch,    OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(data,     OUTPUT);
  pinMode(DIG3,     OUTPUT);
  pinMode(DIG4,     OUTPUT);

  // Both digits off (HIGH = off for common-cathode)
  digitalWrite(DIG3, HIGH);
  digitalWrite(DIG4, HIGH);
}

void loop() {
  unsigned long now = millis();

  // --- Multiplex: alternate between tens and units every MUX_INTERVAL ms ---
  if (now - lastMux >= MUX_INTERVAL) {
    lastMux = now;

    // Blank both before switching to avoid ghosting
    digitalWrite(DIG3, HIGH);
    digitalWrite(DIG4, HIGH);

    if (muxDigit == 0) {
      shiftSeg(table[count / 10]);
      digitalWrite(DIG3, LOW);   // tens digit ON
    } else {
      shiftSeg(table[count % 10]);
      digitalWrite(DIG4, LOW);   // units digit ON
    }

    muxDigit ^= 1;
  }

  // --- Increment counter every COUNT_INTERVAL ms ---
  if (now - lastCount >= COUNT_INTERVAL) {
    lastCount = now;
    if (++count > 99) count = 0;
  }
}
