/*
 * filter.c
 *
 *  Created on: May 18, 2025
 *      Author: USER
 */


#include "filter.h"
static uint8_t begin_filter_flag = 0;
static uint8_t hcsr04_front_filtered_distance;
static uint8_t hcsr04_left45_filtered_distance;
static uint8_t hcsr04_right45_filtered_distance;
static uint8_t hcsr04_left90_filtered_distance;
static uint8_t hcsr04_right90_filtered_distance;

uint8_t *get_hcsr04_front_filtered_distance_ptr()
{
   return &hcsr04_front_filtered_distance;
}
uint8_t *get_hcsr04_left45_filtered_distance_ptr()
{
   return &hcsr04_left45_filtered_distance;
}
uint8_t *get_hcsr04_right45_filtered_distance_ptr()
{
   return &hcsr04_right45_filtered_distance;
}
uint8_t *get_hcsr04_left90_filtered_distance_ptr()
{
   return &hcsr04_left90_filtered_distance;
}
uint8_t *get_hcsr04_right90_filtered_distance_ptr()
{
   return &hcsr04_right90_filtered_distance;
}
