#include "LineFollower.h"
#include "BLE_Control.h"
#include <QTRSensors.h>
#include <Preferences.h>
#include <AsyncTCP.h>
#include <Button.h>

#define IN1 20
#define IN2 19
#define IN3 17
#define IN4 18

#define RED 14
#define GREEN 15
#define BLUE 16

const int TARGET_VALUE = 7500;

float baseSpeed = 100;
int maxSpeed = 254;
bool running = false;

float kp = 0.02;
float ki = 0;
float kd = 0.45;

float p;
float i;
float d;
int correction;
float lastError;

bool readBlack = true;
bool flipMotors = false;

Button button(12);

const int PWM_FREQ = 500;     // Recall that Arduino Uno is ~490 Hz. Official ESP32 example uses 5,000Hz
const int PWM_RESOLUTION = 8;

Preferences prefs;

QTRSensors qtr;
const uint8_t sensorCount = 16;
uint16_t sensorValues[sensorCount];

bool calibrating = false;

void setup() {
  button.begin();
  pinMode(BLUE, OUTPUT);
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);

  initLedc();

  // stop all motors
  drive(0, 0);

  Serial.begin(9600);
  //while(!Serial);
  Serial.println("Serial initialized");

  readCoefficients();

  setupBLE();

  qtr.setTypeRC();
  qtr.setSensorPins((const uint8_t[]){22, 21, 24, 23, 10, 11, 8, 9, 6, 7, 1, 5, 0, 2, 3, 4}, sensorCount);

  if (button.pressed()) {
    calibrating = true;
    while (!button.released());
  } else {
    light(RED);
  }

  if (calibrating) {
    while (!button.pressed());
    calibrateSensors();
  } else {
    readCalibration();
    // go to loop (wait for start from button or bluetooth)
  }

  /*
  while (!button.pressed());

  if (calibrating) {
    calibrateSensors();
  } else {
    readCalibration();
    start();
  }*/
}

void loop() {
  correction = PIDController(qtr.readLineBlack(sensorValues));

  if (running) {
    drive(constrain(baseSpeed + correction, -254, 254), constrain(baseSpeed - correction, -254, 254));
  } else {
    drive(0, 0);
  }

  if (button.pressed()) {
    if (running) {
      stop();
    } else {
      start();
    }
  }
}

void start() {
  if (calibrating) {
    return;
  }
  running = true;
  light(GREEN);
}

void stop() {
  if (calibrating) {
    return;
  }
  running = false;
  light(RED);
}

void calibrateSensors() {
  
  light(BLUE);

  for (uint16_t i = 0; i < 400; i++){
    qtr.calibrate();
  }

  // save to prefs
  if (qtr.calibrationOn.initialized) {
    prefs.begin("prefs", false);
    for (int i = 0; i < sensorCount; i++) {
      String maxKey = "max";
      String minKey = "min";
      maxKey += i;
      minKey += i;
      prefs.putUShort(maxKey.c_str(), qtr.calibrationOn.maximum[i]);
      prefs.putUShort(minKey.c_str(), qtr.calibrationOn.minimum[i]);
    }
    prefs.end();
  }

  light(RED);
  calibrating = false;
}

int PIDController(int value) {
  p = TARGET_VALUE - value;
  i += p;
  d = p - lastError;
  lastError = p;
  return kp * p + ki * i + kd * d;
}

void initLedc() {
  ledcSetup(1, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(2, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(3, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(4, PWM_FREQ, PWM_RESOLUTION);

  ledcAttachPin(IN1,1); 
  ledcAttachPin(IN2,2);
  ledcAttachPin(IN3,3);
  ledcAttachPin(IN4,4);
}

void readCoefficients() {
  prefs.begin("prefs", true); // true = read only
  if (prefs.isKey("bs")) {
    baseSpeed = prefs.getInt("bs");
  }
  if (prefs.isKey("ms")) {
    maxSpeed = prefs.getInt("ms");
  }
  if (prefs.isKey("kp")) {
    kp = prefs.getFloat("kp");
  }
  if (prefs.isKey("ki")) {
    ki = prefs.getFloat("ki");
  }
  if (prefs.isKey("kd")) {
    kd = prefs.getFloat("kd");
  }
  if (prefs.isKey("rb")) {
    readBlack = prefs.getBool("rb");
  }
  if (prefs.isKey("f")) {
    flipMotors = prefs.getBool("f");
  }
  prefs.end();
}

void readCalibration() {
  // dummy calibrate to create arrays
  qtr.calibrate();

  if (!qtr.calibrationOn.initialized) {
    return;
  }

  prefs.begin("prefs", true);
  for (int i = 0; i < sensorCount; i++) {
    String maxKey = "max";
    String minKey = "min";
    maxKey += i;
    minKey += i;
    if (prefs.isKey(maxKey.c_str())) {
      qtr.calibrationOn.maximum[i] = prefs.getUShort(maxKey.c_str());
    }
    if (prefs.isKey(minKey.c_str())) {
      qtr.calibrationOn.minimum[i] = prefs.getUShort(minKey.c_str());
    }
  }
  prefs.end();
}

void writeCoefficients() {
  prefs.begin("prefs", false); // false = read/write
  prefs.putInt("bs", baseSpeed);
  prefs.putInt("ms", maxSpeed);
  prefs.putFloat("kp", kp);
  prefs.putFloat("ki", ki);
  prefs.putFloat("kd", kd);
  prefs.putBool("rb", readBlack);
  prefs.putBool("f", flipMotors);
  prefs.end();
}

void drive(int leftMotor, int rightMotor) {
  if (flipMotors) {
    //driveInverted(leftMotor, rightMotor);
    //return;
  }

  if (leftMotor < 0) {
    // backward
    ledcWrite(1, abs(leftMotor));
    ledcWrite(2, 0);
  } else {
    // forward
    ledcWrite(1, 0);
    ledcWrite(2, leftMotor);
  }

  if (rightMotor < 0) {
    // backward
    ledcWrite(3, abs(rightMotor));
    ledcWrite(4, 0);
  } else {
    // forward
    ledcWrite(3, 0);
    ledcWrite(4, rightMotor);
  }
}

void driveInverted(int leftMotor, int rightMotor) {

  if (leftMotor < 0) {
    // backward
    ledcWrite(2, abs(leftMotor));
    ledcWrite(1, 0);
  } else {
    // forward
    ledcWrite(2, 0);
    ledcWrite(1, leftMotor);
  }

  if (rightMotor < 0) {
    // backward
    ledcWrite(4, abs(rightMotor));
    ledcWrite(3, 0);
  } else {
    // forward
    ledcWrite(4, 0);
    ledcWrite(3, rightMotor);
  }
}

void light(int pin) {
  // RED, GREEN, BLUE: LOW = ON!   HIGH = OFF!
  if (pin == RED) {
    digitalWrite(RED, LOW);
    digitalWrite(GREEN, HIGH);
    digitalWrite(BLUE, HIGH);
  } else if (pin == GREEN) {
    digitalWrite(GREEN, LOW);
    digitalWrite(RED, HIGH);
    digitalWrite(BLUE, HIGH);
  } else if (pin == BLUE) {
    digitalWrite(BLUE, LOW);
    digitalWrite(RED, HIGH);
    digitalWrite(GREEN, HIGH);
  } else if (pin == 0) {
    // all off
    digitalWrite(RED, HIGH);
    digitalWrite(GREEN, HIGH);
    digitalWrite(BLUE, HIGH);
  }
}