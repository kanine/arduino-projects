// Counter 0000 -> 9999, looping, updating 4x per second
// 74HC595 shift register on pins 8/9/10
// DIG1..DIG4 map left-to-right on pins 2..5
// Common-cathode display: digit pin LOW = ON

const int latch    = 9;
const int clockPin = 10;
const int data     = 8;

const int DIG1 = 2;   // leftmost (kept OFF in this test)
const int DIG2 = 3;
const int DIG3 = 4;
const int DIG4 = 5;   // rightmost

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
  shiftSeg(table[value]);    // pre-load segments while previous digit still on
  allDigitsOff();            // dark gap now only ~16 µs (no shiftOut in between)
  digitalWrite(digPin, LOW); // immediately activate next digit
  delayMicroseconds(4000);
}

void setup() {
  pinMode(latch,    OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(data,     OUTPUT);
  pinMode(DIG1,     OUTPUT);
  pinMode(DIG2,     OUTPUT);
  pinMode(DIG3,     OUTPUT);
  pinMode(DIG4,     OUTPUT);

  allDigitsOff();
}

void loop() {
  static unsigned int count = 0;
  static unsigned long lastStep = 0;

  // Split count into individual digits
  byte d1 = count / 1000;
  byte d2 = (count / 100) % 10;
  byte d3 = (count / 10)  % 10;
  byte d4 = count % 10;

  // Multiplex all 4 digits
  showDigit(DIG1, d1);
  showDigit(DIG2, d2);
  showDigit(DIG3, d3);
  showDigit(DIG4, d4);

  // Advance counter 4 times per second
  unsigned long now = millis();
  if (now - lastStep >= 250) {
    lastStep = now;
    count++;
    if (count > 9999) count = 0;
  }
}
