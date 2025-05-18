/*
 * filter.h
 *
 *  Created on: May 18, 2025
 *      Author: USER
 */

#ifndef INC_FILTER_H_
#define INC_FILTER_H_

#include "main.h"
#include "ultrasonic.h"


uint8_t *get_hcsr04_front_filtered_distance_ptr();
uint8_t *get_hcsr04_left45_filtered_distance_ptr();
uint8_t *get_hcsr04_right45_filtered_distance_ptr();
uint8_t *get_hcsr04_left90_filtered_distance_ptr();
uint8_t *get_hcsr04_right90_filtered_distance_ptr();


#endif /* INC_FILTER_H_ */
