#include <Arduino.h>

#include "DvMotor.h"
#include "SerialPrintf.h"

// globals
unsigned long gCount = 0;    // loop counter
int gPushingDir = NT;           // the direction from current state - using for filtering noise
bool gHandleNextPush = true;
unsigned long gPressStart = 0;
unsigned long gMotorStart = 0;
unsigned long gMotorBlinkTimeout = 3000;  // ms - blink light before timeout
unsigned long gMotorTimeout = 35000; // ms - bed posts are 30s
unsigned long gMinPressTime = 50; // ms
int gDir = NT;      // current direction

DvMotor motor;

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
    motor.stop();
  } else if (dir == CW) {
    gMotorStart = ts;
    motor.forward();
  } else if (dir == CCW) {
    gMotorStart = ts;
    motor.reverse();
  }
  setLights(dir);
  gDir = dir;
}

void loop() {
  gCount++;
  unsigned long ts = millis();

  // check for motor timeout if we are on
  if (gMotorStart != 0) {
    unsigned long onTime = ts - gMotorStart;
    if (onTime <= gMotorTimeout && onTime >= (gMotorTimeout - gMotorBlinkTimeout)) {
      serial_printf(Serial, "%l - motor near timeout = %l ms\n", gCount, (ts - gMotorStart));
      blinkLights();
    } else if (onTime > gMotorTimeout) {
      serial_printf(Serial, "%l - motor timeout = %l ms\n", gCount, (ts - gMotorStart));
      setDirection(NT, ts);
      return;
    }
  }

  // read pins
  int newDir = NT;
  int fwd = !digitalRead(SWC_FWD);
  int rev = !digitalRead(SWC_REV);

  // set direction - rev has priority
  if (fwd) newDir = CW;
  if (rev) newDir = CCW;

  if (gPressStart == 0 || gPushingDir != newDir) {     // nothing pressed or a changed
    // set on FWD, REV, NT (treat no press same so we can filter noise from the stop)
//    serial_printf(Serial, "%l - state change = %l - %d\n", gCount, newDir);
    gPressStart = ts;
    gPushingDir = newDir;
    return;
  } else if ((ts - gPressStart) < gMinPressTime) {  // filter change until duration confirms
//    serial_printf(Serial, "%l - noise filter = %l ms\n", gCount, (ts - gPressStart));
    return;
  }
  // reset the filter clock
  gPressStart = 0;
  if (newDir == NT) {
    gHandleNextPush = true;
    return;
  }
  serial_printf(Serial, "%l - push = %d, handle next - %d\n", gCount, newDir, gHandleNextPush);
  if (gHandleNextPush) {
    if (gDir == newDir) setDirection(NT, ts);
    else setDirection(newDir, ts);
    gHandleNextPush = false;
  }  
}