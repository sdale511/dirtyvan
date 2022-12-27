#include <Arduino.h>

#include "L298N.h"
#include "CytronMotorDriver.h"

const int MOTOR_CYTRON = 0;
const int MOTOR_LN298 = 1;
//const int MOTOR_TYPE = MOTOR_CYTRON;
const int MOTOR_TYPE = MOTOR_LN298;

const int CCW = 2; // counter clockwise
const int CW  = 1; // clockwise
const int NT  = 0; // not turning
const int UNSET = -1;  // unset

// globals
int gCount = 0;    // loop counter
int dir = NT;      // current direction
bool inPress = false;  // is button currently pressed
bool latch = true;    // latch mode - light stays on until second push

const int IN1 = 2;
const int IN2 = 4;
const int ENA = 3;
const int SPEED = 127;

L298N motor(ENA, IN1, IN2);
CytronMD cmotor(PWM_DIR, 3, 2);  // PWM = Pin 3, DIR = Pin 2.

volatile int gSwitchOn = LOW;
void switchPressed()
{
  gSwitchOn = digitalRead(2);
  Serial.print("interupt: 2 - ");
  Serial.println(gSwitchOn);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(12, INPUT_PULLUP);
  pinMode(11, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(2), &switchPressed, CHANGE);  // Uno,nano: 2,3 only, Uno Wifi: all, Zero: all but 4
  motor.setSpeed(SPEED);  // 255 full
  Serial.println("setup complete");
}

void loop() {
//  Serial.println(hello);
  gCount++;
  int newDir = UNSET;
  int fwd = digitalRead(12);
  int rev = digitalRead(11);
  
  if (inPress && fwd == HIGH && rev == HIGH) {
    Serial.print("end press: ");
    Serial.println(dir);
    inPress = false;
    delay(100);
    if (latch) return;  // remove for moment only
  }
 if (inPress) {
    delay(100);
    return;
  }

  if (fwd == HIGH && rev == HIGH) {
    if (!latch) newDir = NT;
  } else if (fwd == LOW && rev == LOW) {
    // no-opp
  } else if (fwd == LOW) {
    Serial.println("press fwd");
    inPress = true;
    if (dir == CW) newDir = NT;
    else newDir = CW;
  } else if (rev == LOW) {
    Serial.println("press rev");
    inPress = true;
    if (dir == CCW) newDir = NT;
    else newDir = CCW;
  }

  if (newDir == UNSET || newDir == dir) return; // no change;

  Serial.print(gCount);
  Serial.print(" fwd: ");
  Serial.print(fwd);
  Serial.print(" rev: ");
  Serial.print(rev);
  Serial.print(" dir: ");
  Serial.println(newDir);

  if (newDir == NT) {
    if (MOTOR_TYPE == MOTOR_LN298) motor.stop();
    if (MOTOR_TYPE == MOTOR_CYTRON) cmotor.setSpeed(0);
    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(10, LOW);
  } else if (newDir == CW) {
    if (MOTOR_TYPE == MOTOR_LN298) motor.forward();
    if (MOTOR_TYPE == MOTOR_CYTRON) cmotor.setSpeed(255);
    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(10, LOW);
  } else if (newDir == CCW) {
    if (MOTOR_TYPE == MOTOR_LN298) motor.backward();
    if (MOTOR_TYPE == MOTOR_CYTRON) cmotor.setSpeed(-255);
    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(10, HIGH);
  }
  dir = newDir;
  delay(100);
}