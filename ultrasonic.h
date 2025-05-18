/*
 * ultrasonic.h
 *
 *  Created on: May 17, 2025
 *      Author: USER
 */

#ifndef INC_ULTRASONIC_H_
#define INC_ULTRASONIC_H_

#include "main.h"
#include "tim.h"
#include "filter.h"

#define HCSR04_TRIG_GPIO_PORT   GPIOB
#define HCSR04_TRIG_PIN         GPIO_PIN_12
#define SENSOR_NUM 2
/*TIM1 CH1 HC-SR04 ( FRONT )
 *TIM1 CH2 HC-SR04 ( LEFT  45 )
 *TIM1 CH3 HC-SR04 ( RIGHT 45 )
 *TIM1 CH4 HC-SR04 ( LEFT  90 )
 *TIM3 Ch1 HC-SR04 ( RIGHT 90 )
 */
typedef struct
{
  uint16_t ic_value_1;
  uint16_t ic_value_2;
  uint16_t echo_time;
  uint16_t distance;
  uint8_t capture_flag;
  TIM_HandleTypeDef *htim;
  uint32_t channel;
  uint8_t sensoring_count;
  uint8_t filter_flag;
}HCSR04;

void ultra_init(HCSR04 *hcsr04, TIM_HandleTypeDef *htim, uint32_t channel);
void module_init();
void ultra_Trigger(); //5 modules share the same trigger port
void ultra_process(HCSR04 *hcsr04, uint32_t TIM_IT_CCx, uint8_t* filtered_distance);

HCSR04 * get_hcsr04_front_ptr();
HCSR04 * get_hcsr04_left45_ptr();
HCSR04 * get_hcsr04_right45_ptr();
HCSR04 * get_hcsr04_left90_ptr();
HCSR04 * get_hcsr04_right90_ptr();





#endif /* INC_ULTRASONIC_H_ */
