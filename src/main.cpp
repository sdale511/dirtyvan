#include <Arduino.h>

#include "L298N.h"
#include "CytronMotorDriver.h"
#include "SerialPrintf.h"

const int MOTOR_CYTRON = 0;
const int MOTOR_LN298 = 1;
//const int MOTOR_TYPE = MOTOR_CYTRON;
const int MOTOR_TYPE = MOTOR_LN298;

const int CCW = 2; // counter clockwise
const int CW  = 1; // clockwise
const int NT  = 0; // not turning
const int UNSET = -1;  // unset

// globals
long gCount = 0;    // loop counter
int dir = NT;      // current direction
bool inPress = false;  // is button currently pressed
bool latch = true;    // latch mode - light stays on until second push

const int PIN_PWM = 5;

const int PIN_FWD = 6;
const int PIN_REV = 7;
L298N motor(PIN_PWM, PIN_FWD, PIN_REV);

const int PIN_CYTRON_PWM = 3;
const int PIN_CYTRON_DIR = 2;
CytronMD cmotor(PWM_DIR, PIN_CYTRON_PWM, PIN_CYTRON_DIR);

const int SPEED = 255;  // 255 full

const int LED_FWD = 13;
const int LED_REV = 12;
const int SWC_FWD = 2;
const int SWC_REV = 3;

const int END_LOOP_DELAY = 100;

volatile int gFwd = LOW;
volatile int gRev = LOW;
void switchPressed()
{
  gFwd = digitalRead(SWC_FWD);
  gRev = digitalRead(SWC_REV);
//  serial_printf(Serial, "interupt: FWD = %d, REV = %d\n", gFwd, gRev);
}

void setup() {
  // put your setup code here, to run once:
  gCount = 0;
  Serial.begin(9600);
  pinMode(LED_FWD, OUTPUT);
  pinMode(LED_REV, OUTPUT);
  pinMode(SWC_FWD, INPUT_PULLUP);
  pinMode(SWC_REV, INPUT_PULLUP);
//  attachInterrupt(digitalPinToInterrupt(SWC_FWD), &switchPressed, CHANGE);  // Uno,nano: 2,3 only, Uno Wifi: all, Zero: all but 4
//  attachInterrupt(digitalPinToInterrupt(SWC_REV), &switchPressed, CHANGE);  // Uno,nano: 2,3 only, Uno Wifi: all, Zero: all but 4
  if (MOTOR_TYPE == MOTOR_LN298) motor.setSpeed(SPEED);
  serial_printf(Serial, "setup complete: motor: %s\n", (MOTOR_TYPE == MOTOR_LN298 ? "LN298" : "CYTRON"));
}

void loop() {
  gCount++;
  int newDir = UNSET;
  int fwd = digitalRead(SWC_FWD);
  int rev = digitalRead(SWC_REV);
  
  if (inPress && fwd == HIGH && rev == HIGH) {
    serial_printf(Serial, "%l - end press dir = %d\n", gCount, dir);
    inPress = false;
    delay(100);
    if (latch) return;  // remove for moment only
  }
 if (inPress) {
    delay(100);
    return;
  }

  if (fwd == HIGH && rev == HIGH) {
    if (!latch)  {
      newDir = NT;
      serial_printf(Serial, "%l - moment switch off, newDir = %d\n", gCount, newDir);
    }
  } else if (fwd == LOW && rev == LOW) {
    // no-opp
  } else if (fwd == LOW) {
    inPress = true;
    if (dir == CW) newDir = NT;
    else newDir = CW;
    serial_printf(Serial, "%l - press fwd, newDir = %d\n", gCount, newDir);
  } else if (rev == LOW) {
    inPress = true;
    if (dir == CCW) newDir = NT;
    else newDir = CCW;
    serial_printf(Serial, "%l - press rev, newDir = %d\n", gCount, newDir);
  }

  if (newDir == UNSET || newDir == dir) return; // no change;

  serial_printf(Serial, "%l - fwd = %d, rev = %d, dir = %d, newDir = %d\n", gCount, fwd, rev, dir, newDir);

  if (newDir == NT) {
    if (MOTOR_TYPE == MOTOR_LN298) motor.stop();
    if (MOTOR_TYPE == MOTOR_CYTRON) cmotor.setSpeed(0);
    digitalWrite(LED_FWD, LOW);
    digitalWrite(LED_REV, LOW);
  } else if (newDir == CW) {
    if (MOTOR_TYPE == MOTOR_LN298) motor.forward();
    if (MOTOR_TYPE == MOTOR_CYTRON) cmotor.setSpeed(255);
    digitalWrite(LED_FWD, HIGH);
    digitalWrite(LED_REV, LOW);
  } else if (newDir == CCW) {
    if (MOTOR_TYPE == MOTOR_LN298) motor.backward();
    if (MOTOR_TYPE == MOTOR_CYTRON) cmotor.setSpeed(-255);
    digitalWrite(LED_FWD, LOW);
    digitalWrite(LED_REV, HIGH);
  }
  dir = newDir;
  delay(END_LOOP_DELAY);  // removes any double sends from up or down switches
}