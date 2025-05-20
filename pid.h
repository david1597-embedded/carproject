/*
 * pid.h
 *
 *  Created on: May 19, 2025
 *      Author: USER
 */

#ifndef INC_PID_H_
#define INC_PID_H_

#include "main.h"
#include "motor.h"
#include "filter.h"

#define MAX_CCR 400
#define MIN_CCR 200
#define SCALE_FACTOR 100
#define FRONT_THRESHOLD 30
#define MIN_FRONT_DISTANCE 5
#define MIN_WALL_DISTANCE 3
#define PID_GAIN 2
#define CAR_WIDTH 16

typedef struct {
  float Kp;
  float Ki;
  float Kd;
  int8_t integral;
  int8_t prev_error;
  uint16_t dt;
}PIDController;


void pid_init(PIDController *pid, float Kp, float Ki, float Kd, uint16_t dt );
void pid_process();
void pid_module_init();
void pid_reset_integral();
uint16_t pid_compute(PIDController *pid, int8_t error);

#endif /* INC_PID_H_ */
