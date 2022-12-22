
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
//  Serial.begin(115200);
  Serial.begin(9600);
  motor.begin();
  //L298N DC Motor by Robojax.com

}

void loop() {
  
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
