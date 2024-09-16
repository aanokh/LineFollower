#ifndef LINEFOLLOWER_H
#define LINEFOLLOWER_H

extern float kp;
extern float ki;
extern float kd;

extern float baseSpeed;
extern bool running;

void writeCoefficients();
void start();
void stop();

#endif