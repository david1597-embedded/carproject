/*
 * ultrasonic.c
 *
 *  Created on: May 17, 2025
 *      Author: USER
 */

#include "ultrasonic.h"
#include <stdio.h>
/*channel interrupt flags*/
static HCSR04 hcsr04_front;
static HCSR04 hcsr04_left45;
static HCSR04 hcsr04_right45;
static HCSR04 hcsr04_left90;
static HCSR04 hcsr04_right90;



//static HCSR04 *sensors[SENSOR_NUM] = {0};

void ultra_init(HCSR04 *hcsr04, TIM_HandleTypeDef *htim, uint32_t channel)
{
   hcsr04->ic_value_1  = 0;
   hcsr04->ic_value_2  = 0;
   hcsr04->distance    = 0;
   hcsr04->capture_flag= 0;
   hcsr04->echo_time   = 0;
   hcsr04->htim        = htim;
   hcsr04->channel = channel;
   hcsr04->sensoring_count = 0;
   hcsr04->filter_flag = 0;
}


void module_init()
{
   ultra_init(&hcsr04_front  , &htim1, TIM_CHANNEL_1);
   ultra_init(&hcsr04_left45 , &htim1, TIM_CHANNEL_2);
   ultra_init(&hcsr04_right45, &htim1, TIM_CHANNEL_3);
   ultra_init(&hcsr04_left90 , &htim1, TIM_CHANNEL_4);
   ultra_init(&hcsr04_right90, &htim3, TIM_CHANNEL_1);
}

void ultra_Trigger()
{

    /*Turn on the trigger Timer*/
    printf("Pulse Triggered! \n");
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5 , GPIO_PIN_RESET);
    __HAL_TIM_SET_COUNTER(&htim4, 0);
    while (__HAL_TIM_GET_COUNTER(&htim4) < 1);

    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_SET);
    __HAL_TIM_SET_COUNTER(&htim4, 0);
    while (__HAL_TIM_GET_COUNTER(&htim4) < 10);


    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET);
    __HAL_TIM_SET_COUNTER(&htim4, 0);

    if(hcsr04_front.capture_flag == 0 && hcsr04_left45.capture_flag == 0 && hcsr04_right45.capture_flag == 0
	&& hcsr04_left90.capture_flag == 0 && hcsr04_right90.capture_flag == 0)
    {
      __HAL_TIM_ENABLE_IT(&htim1, TIM_IT_CC1);
      __HAL_TIM_ENABLE_IT(&htim1, TIM_IT_CC2);
      __HAL_TIM_ENABLE_IT(&htim1, TIM_IT_CC3);
      __HAL_TIM_ENABLE_IT(&htim1, TIM_IT_CC4);
      __HAL_TIM_ENABLE_IT(&htim3, TIM_IT_CC1);
    }

   // printf("Interrupt Enabled! \n");

    // __HAL_TIM_ENABLE_IT(&htim1, TIM_CHANNEL_2);



}

HCSR04 * get_hcsr04_front_ptr()
{
  return &hcsr04_front;
}

HCSR04 * get_hcsr04_left45_ptr()
{
  return &hcsr04_left45;
}

HCSR04 * get_hcsr04_right45_ptr()
{
  return &hcsr04_right45;
}

HCSR04 * get_hcsr04_left90_ptr()
{
  return &hcsr04_left90;
}

HCSR04 * get_hcsr04_right90_ptr()
{
  return &hcsr04_right90;
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  if(htim->Instance == TIM1)
  {
    if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
    {
      // printf("Interrupt activated!! \n");
      //printf("%lu Hz \n", SystemCoreClock);
      if(hcsr04_front.filter_flag == 0)
      {
	  ultra_process(&hcsr04_front, TIM_IT_CC1, get_hcsr04_front_filtered_distance_ptr());
      }
    }
    if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
    {
      if(hcsr04_left45.filter_flag == 0)
      {
	  ultra_process(&hcsr04_left45, TIM_IT_CC2, get_hcsr04_left45_filtered_distance_ptr());
      }
    }
    if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3)
    {
      if(hcsr04_right45.filter_flag == 0)
      {
	  ultra_process(&hcsr04_right45, TIM_IT_CC3, get_hcsr04_right45_filtered_distance_ptr());
      }
    }
    if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4)
    {
      if(hcsr04_left90.filter_flag == 0)
      {
	  ultra_process(&hcsr04_left90, TIM_IT_CC4, get_hcsr04_left90_filtered_distance_ptr());
      }
    }
  }
  if(htim->Instance == TIM3)
  {
    if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
    {
      if(hcsr04_right90.filter_flag == 0)
      {
	   ultra_process(&hcsr04_right90, TIM_IT_CC1, get_hcsr04_right90_filtered_distance_ptr());
      }
    }
  }

}

void ultra_process(HCSR04 * hcsr04, uint32_t TIM_IT_CCx, uint8_t * filtered_distance)
{
   if(hcsr04->capture_flag == 0)
   {
       hcsr04->ic_value_1 = HAL_TIM_ReadCapturedValue(hcsr04->htim, hcsr04->channel);
       hcsr04->capture_flag = 1;
       __HAL_TIM_SET_CAPTUREPOLARITY(hcsr04->htim, hcsr04->channel, TIM_INPUTCHANNELPOLARITY_FALLING);
   }
   else if(hcsr04->capture_flag == 1)
   {
      uint32_t diff;
       hcsr04->ic_value_2 = HAL_TIM_ReadCapturedValue(hcsr04->htim, hcsr04->channel);
       if(hcsr04->ic_value_2 > hcsr04->ic_value_1)
       {
	 diff = hcsr04->ic_value_2 - hcsr04->ic_value_1;
       }
       else
       {
	 diff = (0xFFFF - (hcsr04->ic_value_1)) + hcsr04->ic_value_2 + 1;
       }

       if( diff >= 150 && diff <= 23500)
       {
	 hcsr04->echo_time = diff;
	 hcsr04->distance = hcsr04->echo_time /58 ;
       }
       else
       {
	 hcsr04->echo_time = 0;
	 hcsr04->distance = -1; // 오류 처리
       }
      // printf("IC_VAUE 1 : %d, IC_VALUE 2 %d, diff: %d \n", hcsr04->ic_value_1, hcsr04->ic_value_2, hcsr04->ic_value_2 - hcsr04->ic_value_1);
      // hcsr04->distance = hcsr04->echo_time / 58;
      // printf("echo_time : %d , distance : %d \n",hcsr04->echo_time, hcsr04->distance );

       hcsr04->sensoring_count = hcsr04->sensoring_count + 1;
       *filtered_distance = *filtered_distance + hcsr04 -> distance;
       if(hcsr04->sensoring_count == 3)
       {
	 hcsr04->filter_flag = 1;
       }
       hcsr04->capture_flag = 0;
       __HAL_TIM_SET_CAPTUREPOLARITY(hcsr04->htim, hcsr04->channel, TIM_INPUTCHANNELPOLARITY_RISING);
       __HAL_TIM_DISABLE_IT(hcsr04->htim, TIM_IT_CCx);
   }
}


