/*
 * motor.c
 *
 *  Created on: May 17, 2025
 *      Author: USER
 */


#include "motor.h"

void motor_move(uint8_t left_motor_direction, uint8_t right_motor_direction)
{
  /* 0 : Stop
   * 1 : Forward
   * 2 : Backward
   */
   switch(left_motor_direction)
   {
     case 0:
       HAL_GPIO_WritePin(IN3_GPIO_PORT, IN3_PIN, GPIO_PIN_SET);
       HAL_GPIO_WritePin(IN4_GPIO_PORT, IN4_PIN, GPIO_PIN_SET);
       break;
     case 1:
       HAL_GPIO_WritePin(IN3_GPIO_PORT, IN3_PIN, GPIO_PIN_RESET);
       HAL_GPIO_WritePin(IN4_GPIO_PORT, IN4_PIN, GPIO_PIN_SET);
       break;
     case 2:
       HAL_GPIO_WritePin(IN3_GPIO_PORT, IN3_PIN, GPIO_PIN_SET);
       HAL_GPIO_WritePin(IN4_GPIO_PORT, IN4_PIN, GPIO_PIN_RESET);
       break;

   }

   switch(right_motor_direction)
   {
    case 0:
       HAL_GPIO_WritePin(IN1_GPIO_PORT, IN1_PIN, GPIO_PIN_SET);
       HAL_GPIO_WritePin(IN2_GPIO_PORT, IN2_PIN, GPIO_PIN_SET);
       break;
    case 1:
       HAL_GPIO_WritePin(IN1_GPIO_PORT, IN1_PIN, GPIO_PIN_RESET);
       HAL_GPIO_WritePin(IN2_GPIO_PORT, IN2_PIN, GPIO_PIN_SET);
      break;
    case 2:
       HAL_GPIO_WritePin(IN1_GPIO_PORT, IN1_PIN, GPIO_PIN_SET);
       HAL_GPIO_WritePin(IN2_GPIO_PORT, IN2_PIN, GPIO_PIN_RESET);
       break;

  }


}

void motor_speed(uint16_t left_motor_ccr, uint16_t right_motor_ccr)
{
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, left_motor_ccr);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, right_motor_ccr);
}

