#include <Arduino.h>

#include "L298N.h"
#include "CytronMotorDriver.h"
#include "RobojaxBTS7960.h"
#include "SerialPrintf.h"

const int MOTOR_CYTRON = 0;
const int MOTOR_LN298 = 1;
const int MOTOR_BTS7960 = 2;
//const int MOTOR_TYPE = MOTOR_CYTRON;
//const int MOTOR_TYPE = MOTOR_LN298;
const int MOTOR_TYPE = MOTOR_BTS7960;

const int CCW = 2; // counter clockwise
const int CW  = 1; // clockwise
const int NT  = 0; // not turning
const int UNSET = -1;  // unset

// globals
unsigned long gCount = 0;    // loop counter
int gLastPressDir = NT;
unsigned long gPressStart = 0;
unsigned long gMotorStart = 0;
unsigned long gMotorBlinkTimeout = 3000;  // ms - blink light before timeout
unsigned long gMotorTimeout = 35000; // ms - bed posts are 30s
unsigned long gMinPressTime = 50; // ms
int gDir = NT;      // current direction

const int PIN_PWM = 6;
const int PIN_FWD = 7;
const int PIN_REV = 8;
L298N motor(PIN_PWM, PIN_FWD, PIN_REV);

const int PIN_CYTRON_PWM = 3;
const int PIN_CYTRON_DIR = 2;
CytronMD cmotor(PWM_DIR, PIN_CYTRON_PWM, PIN_CYTRON_DIR);

#define RPWM 5
#define R_EN 4
#define R_IS 3
#define LPWM 6
#define L_EN 7
#define L_IS 8
#define debug 1 //change to 0 to hide serial monitor debugging infornmation or set to 1 to view
RobojaxBTS7960 bmotor(R_EN, RPWM, R_IS, L_EN, LPWM, L_IS, debug);

const int SPEED = 255;  // 255 full

const int LED_FWD = 9;
const int LED_REV = 10;
const int SWC_FWD = 11;
const int SWC_REV = 12;

volatile int gFwd = LOW;
volatile int gRev = LOW;
// warning: you still get multiple up or down interrupts from the switches
void switchPressed()
{
  gFwd = digitalRead(SWC_FWD);
  gRev = digitalRead(SWC_REV);
  serial_printf(Serial, "interupt: FWD = %d, REV = %d\n", gFwd, gRev);
}

void setup() {
  // put your setup code here, to run once:
  gCount = 0;
  gMotorStart = 0;
  Serial.begin(9600);
  pinMode(LED_FWD, OUTPUT);
  pinMode(LED_REV, OUTPUT);
  pinMode(SWC_FWD, INPUT_PULLUP);
  pinMode(SWC_REV, INPUT_PULLUP);
// sticking with loop logic as interrupts still give multiple events up or down  
//  attachInterrupt(digitalPinToInterrupt(SWC_FWD), &switchPressed, CHANGE);  // Uno,nano: 2,3 only, Uno Wifi: all, Zero: all but 4
//  attachInterrupt(digitalPinToInterrupt(SWC_REV), &switchPressed, CHANGE);  // Uno,nano: 2,3 only, Uno Wifi: all, Zero: all but 4
  if (MOTOR_TYPE == MOTOR_LN298) motor.setSpeed(SPEED);
  if (MOTOR_TYPE == MOTOR_BTS7960) bmotor.begin();
  String motorType = "unknown";
  switch(MOTOR_TYPE) {
    case MOTOR_LN298:
    motorType = "LN298";
    break;
    case MOTOR_CYTRON:
    motorType = "CYTRON";
    break;
    case MOTOR_BTS7960:
    motorType = "BTS796";
    break;
  }
  serial_printf(Serial, "setup complete: motor: %s\n", motorType.c_str());
}

void setLights(int dir) {
  if (dir == NT) {
    digitalWrite(LED_FWD, LOW);
    digitalWrite(LED_REV, LOW);
  } else if (dir == CW) {
    digitalWrite(LED_FWD, HIGH);
    digitalWrite(LED_REV, LOW);
  } else if (dir == CCW) {
    digitalWrite(LED_FWD, LOW);
    digitalWrite(LED_REV, HIGH);
  }
}

void blinkLights() {
  setLights(NT);
  delay(500);
  setLights(gDir);
  delay(500);
}

void setDirection(int dir, unsigned long ts) {
  serial_printf(Serial, "%l - setDirection - dir = %d\n", gCount, dir);
  if (dir == NT) {
    gMotorStart = 0;
    if (MOTOR_TYPE == MOTOR_LN298) motor.stop();
    if (MOTOR_TYPE == MOTOR_CYTRON) cmotor.setSpeed(0);
    if (MOTOR_TYPE == MOTOR_BTS7960) bmotor.stop();
  } else if (dir == CW) {
    gMotorStart = ts;
    if (MOTOR_TYPE == MOTOR_LN298) motor.forward();
    if (MOTOR_TYPE == MOTOR_CYTRON) cmotor.setSpeed(255);
    if (MOTOR_TYPE == MOTOR_BTS7960) bmotor.rotate(100, CW);
  } else if (dir == CCW) {
    gMotorStart = ts;
    if (MOTOR_TYPE == MOTOR_LN298) motor.backward();
    if (MOTOR_TYPE == MOTOR_CYTRON) cmotor.setSpeed(-255);
    if (MOTOR_TYPE == MOTOR_BTS7960) bmotor.rotate(100, CCW);
  }
  setLights(dir);
  gDir = dir;
}

void loop() {
  gCount++;
  unsigned long ts = millis();
  int newDir = UNSET;
  int fwd = digitalRead(SWC_FWD);
  int rev = digitalRead(SWC_REV);

  if ((gPressStart != 0) && (fwd == LOW || rev == LOW)) {
    // we are in a press
    return;
  }

  if (gMotorStart != 0) {
    unsigned long onTime = ts - gMotorStart;
    if (onTime <= gMotorTimeout && onTime >= (gMotorTimeout - gMotorBlinkTimeout)) {
      serial_printf(Serial, "%l - motor about timeout = %l ms\n", gCount, (ts - gMotorStart));
      blinkLights();
    } else if (onTime > gMotorTimeout) {
      serial_printf(Serial, "%l - motor timeout = %l ms\n", gCount, (ts - gMotorStart));
      setDirection(NT, ts);
      return;
    }
  }
  
  if (fwd == HIGH && rev == HIGH) {
    if (gPressStart != 0) {
      if ((ts - gPressStart) < gMinPressTime) {
        setLights(gDir);
        serial_printf(Serial, "%l - too quick = %l ms\n", gCount, (ts - gPressStart));
        gPressStart = 0;
        return;
      } else  {
        newDir = gLastPressDir;
        if (gLastPressDir == gDir) newDir = NT;
        serial_printf(Serial, "%l - end press dir = %d\n", gCount, newDir);
        setDirection(newDir, ts);
        gPressStart = 0;
        gLastPressDir = 0;
      }
    }
  } else if (fwd == LOW) {
    gPressStart = ts;
    gLastPressDir = CW;
    setLights(CW);
    serial_printf(Serial, "%l - press down fwd, newDir = %d\n", gCount, gLastPressDir);
  } else if (rev == LOW) {
    gPressStart = ts;
    gLastPressDir = CCW;
    setLights(CCW);
    serial_printf(Serial, "%l - press down rev, newDir = %d\n", gCount, gLastPressDir);
  }  
}