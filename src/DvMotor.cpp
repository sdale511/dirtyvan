#include "DvMotor.h"
#include "SerialPrintf.h"

//#define MOTOR_L298N
//#define MOTOR_CYTRON
#define MOTOR_BTS7960

#ifdef MOTOR_L298N
#include "motors/L298N.h"
const int PIN_PWM = 6;
const int PIN_FWD = 7;
const int PIN_REV = 8;
L298N lmotor(PIN_PWM, PIN_FWD, PIN_REV);
#endif

#ifdef MOTOR_CYTRON
#include "motors/CytronMotorDriver.h"
const int PIN_CYTRON_PWM = 3;
const int PIN_CYTRON_DIR = 2;
CytronMD cmotor(PWM_DIR, PIN_CYTRON_PWM, PIN_CYTRON_DIR);
#endif

#ifdef MOTOR_BTS7960
#include "motors/RobojaxBTS7960.h"
#define RPWM 5
#define R_EN 4
#define R_IS 3
#define LPWM 6
#define L_EN 7
#define L_IS 8
#define debug 1 //change to 0 to hide serial monitor debugging infornmation or set to 1 to view
RobojaxBTS7960 bmotor(R_EN, RPWM, R_IS, L_EN, LPWM, L_IS, debug);
#endif

DvMotor::DvMotor() {

}

void DvMotor::init() {
  String motorType = "";
#ifdef MOTOR_LN298N
  lmotor.setSpeed(SPEED);
  motorType += "LN298";
#endif
#ifdef MOTOR_CYTRON
  motorType += "CYTRON";
#endif
#ifdef MOTOR_BTS7960
  bmotor.begin();
  motorType += "BTS796";
#endif
  serial_printf(Serial, "setup complete: motor: %s\n", motorType.c_str());
}

void DvMotor::forward() {
#ifdef MOTOR_LN298N
  lmotor.forward();
#endif
#ifdef MOTOR_CYTRON
  cmotor.setSpeed(255);
#endif
#ifdef MOTOR_BTS7960
  bmotor.rotate(100, CW);
#endif
}

void DvMotor::reverse() {
#ifdef MOTOR_LN298N
  lmotor.backward();
#endif
#ifdef MOTOR_CYTRON
  cmotor.setSpeed(-255);
#endif
#ifdef MOTOR_BTS7960
  bmotor.rotate(100, CCW);
#endif
}

void DvMotor::stop() {
#ifdef MOTOR_LN298N
  lmotor.stop();
#endif
#ifdef MOTOR_CYTRON
  cmotor.setSpeed(0);
#endif
#ifdef MOTOR_BTS7960
  bmotor.stop();
#endif
}
