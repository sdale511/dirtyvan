#ifndef DVMOTOR_h
#define DVMOTOR_h

const int CCW = 2; // counter clockwise
const int CW  = 1; // clockwise
const int NT  = 0; // not turning
const int SPEED = 255;  // 255 full

const int MOTOR_CYTRON = 0;
const int MOTOR_LN298 = 1;
const int MOTOR_BTS7960 = 2;

class DvMotor {
public:
    DvMotor();
    void init();
    void forward();
    void reverse();
    void stop();
};

#endif
