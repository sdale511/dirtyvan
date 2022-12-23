
#include "Robojax_L298N_DC_motor.h"

// motor 1 settings
#define IN1 2
#define IN2 4
#define ENA 3 // this pin must be PWM enabled pin



const int CCW = 2; // do not change
const int CW  = 1; // do not change

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
int dir = 0;
int inPress = 0;

void loop() {
//  while(1) {
  int newDir = -1;
    int fwd = digitalRead(12);
    int rev = digitalRead(11);
  
    if (inPress == 1 && fwd == HIGH && rev == HIGH) {
      Serial.print("end press: ");
      Serial.println(dir);
      inPress = 0;
    }
    if (inPress == 1) return;

    if (fwd == HIGH && rev == HIGH) {
      newDir = 0;
    } else if (fwd == LOW && rev == LOW) {
      // no-opp
    } else if (fwd == LOW) {
      Serial.println("press fwd");
      inPress = 1;
      newDir = 1;
    } else if (rev == LOW) {
      Serial.println("press rev");
      motor.rotate(motor1, 100, CCW);//run motor1 at 60% speed in CCW direction
      inPress = 1;
      newDir = 2;
    }

    if (newDir == -1 || newDir == dir) return; // no change;

    Serial.print(gCount++);
    Serial.print(" fwd: ");
    Serial.print(fwd);
    Serial.print(" rev: ");
    Serial.print(rev);
    Serial.print(" dir: ");
    Serial.println(newDir);

    if (newDir == 0) {
      motor.brake(1);
      digitalWrite(LED_BUILTIN, LOW);
      digitalWrite(10, LOW);
    } else if (newDir == 1) {
      motor.rotate(motor1, 100, CW);
      digitalWrite(LED_BUILTIN, HIGH);
      digitalWrite(10, LOW);
    } else if (newDir == 2) {
      motor.rotate(motor1, 100, CCW);
      digitalWrite(LED_BUILTIN, LOW);
      digitalWrite(10, HIGH);
    }
    dir = newDir;
    delay(100);

}

void loopDemo() {
  
  motor.demo(1);
  motor.rotate(motor1, 60, CW);//run motor1 at 60% speed in CW direction
 
  delay(3000);

  motor.brake(1);
  delay(2000);


  motor.rotate(motor1, 100, CW);//run motor1 at 60% speed in CW direction
  delay(3000);

  motor.brake(1);
  delay(2000);  

  for(int i=0; i<=100; i++)
  {
    motor.rotate(motor1, i, CW);// turn motor1 with i% speed in CW direction (whatever is i) 
    delay(100);
  }
  delay(2000);
  
  motor.brake(1);
  delay(2000);  

  motor.rotate(motor1, 70, CCW);//run motor1 at 70% speed in CCW direction
  delay(4000);

  motor.brake(1);
  delay(2000);    
  
}
