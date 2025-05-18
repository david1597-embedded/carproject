/*
 * motor.h
 *
 *  Created on: May 17, 2025
 *      Author: USER
 */

#ifndef INC_MOTOR_H_
#define INC_MOTOR_H_

#include "main.h"
#include "tim.h"
#include "gpio.h"
#include "ultrasonic.h"

#define IN1_GPIO_PORT GPIOB
#define IN1_PIN       GPIO_PIN_6
#define IN2_GPIO_PORT GPIOA
#define IN2_PIN       GPIO_PIN_7
#define IN3_GPIO_PORT GPIOC
#define IN3_PIN       GPIO_PIN_8
#define IN4_GPIO_PORT GPIOC
#define IN4_PIN       GPIO_PIN_6

void motor_move(uint8_t left_motor_direction, uint8_t right_motor_direction);// 0: forward direction 1 : backward direction. Direction must be different to move
void motor_speed(uint16_t left_motor_ccr, uint16_t right_motor_ccr);

#endif /* INC_MOTOR_H_ */
