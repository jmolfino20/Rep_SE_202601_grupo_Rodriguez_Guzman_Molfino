#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>

void motor_init(void);

void motor_stop(void);
void motor_forward(uint8_t dutyA, uint8_t dutyB);
void motor_backward(uint8_t dutyA, uint8_t dutyB);

#endif