#include <TM1637Display.h>
#include <Button.h>

#define CLOCK 18
#define IO 19
#define SENSOR 5

Button buttonA(22);
Button buttonB(21);

TM1637Display display(CLOCK, IO);
int secs = 0;
long lastMillis;
long fastestTime = 0;
uint8_t data[4];


void setup() {
  pinMode(17, INPUT);
  buttonA.begin();
  buttonB.begin();
  display.setBrightness(0x0f);
  data[0] = display.encodeDigit(0);
  data[1] = display.encodeDigit(0);
  data[2] = display.encodeDigit(0);
  data[3] = display.encodeDigit(0);
  display.setSegments(data);
  while (!buttonA.pressed());
}

void loop() {
  //display.setSegments(data);
  //display.showNumberDecEx(millis()/10,0b01000000,true);
  if (!digitalRead(SENSOR)) {
    reset();
  }
}

void reset() {
  display.showNumberDecEx((millis() - lastMillis)/10,0b01000000,false);
  if (fastestTime == 0 || (millis() - lastMillis) < fastestTime) {
    fastestTime = millis() - lastMillis;
  }
  lastMillis = millis();
  delay(300);
}