int latch = 9;
int clockPin = 10;
int data = 8;

void setup() {
  pinMode(latch, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(data, OUTPUT);
}

void loop() {
  digitalWrite(latch, LOW);
  shiftOut(data, clockPin, MSBFIRST, 0x7F); // "8" (no DP)
  digitalWrite(latch, HIGH);
  delay(200);
}