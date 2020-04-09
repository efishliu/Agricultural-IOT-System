#ifndef _MOTOR_H_
#define _MOTOR_H_

#define uchar  unsigned char


void InitMotor(void);
void MotorStop(void);
void MotorCCW(uchar Speed);
void MotorCW(uchar Speed);


#endif