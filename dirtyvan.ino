
#include "Robojax_L298N_DC_motor.h"

// motor 1 settings
#define IN1 2
#define IN2 4
#define ENA 3 // this pin must be PWM enabled pin



const int CCW = 2; // counter clockwise
const int CW  = 1; // clockwise
const int NT  = 0; // not turning
const int UNSET = -1;  // unset

#define motor1 1 // do not change


// use the line below for single motor
Robojax_L298N_DC_motor motor(IN1, IN2, ENA, true);


void setup() {
  Serial.begin(9600);
   pinMode(LED_BUILTIN, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(12, INPUT_PULLUP);
  pinMode(11, INPUT_PULLUP);
  motor.begin();
  Serial.println("setup complete");
}

int gCount = 0;
int dir = NT;
bool inPress = false;
bool latch = false;

void loop() {
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

  Serial.print(gCount++);
  Serial.print(" fwd: ");
  Serial.print(fwd);
  Serial.print(" rev: ");
  Serial.print(rev);
  Serial.print(" dir: ");
  Serial.println(newDir);

  if (newDir == NT) {
    motor.brake(1);
    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(10, LOW);
  } else if (newDir == CW) {
    motor.rotate(motor1, 100, CW);
    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(10, LOW);
  } else if (newDir == CCW) {
    motor.rotate(motor1, 100, CCW);
    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(10, HIGH);
  }
  dir = newDir;
  delay(100);
}