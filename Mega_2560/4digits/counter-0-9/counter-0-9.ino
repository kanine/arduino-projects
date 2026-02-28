int latch=9, clockPin=10, data=8;

int DIG1=2, DIG2=3, DIG3=4, DIG4=5;

unsigned char table[] =
{
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

void shiftSeg(byte s){
  digitalWrite(latch, LOW);
  shiftOut(data, clockPin, MSBFIRST, s);
  digitalWrite(latch, HIGH);
}

void setup(){
  pinMode(latch,OUTPUT); pinMode(clockPin,OUTPUT); pinMode(data,OUTPUT);

  pinMode(DIG1,OUTPUT); pinMode(DIG2,OUTPUT); pinMode(DIG3,OUTPUT); pinMode(DIG4,OUTPUT);

  digitalWrite(DIG1,HIGH);
  digitalWrite(DIG2,HIGH);
  digitalWrite(DIG3,HIGH);
  digitalWrite(DIG4,LOW);   // rightmost ON
}

void loop(){
  for(byte d=0; d<=9; d++){
    shiftSeg(table[d]);
    delay(700);
  }
}