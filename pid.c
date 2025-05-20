/*
 * pid.c
 *
 *  Created on: May 19, 2025
 *      Author: USER
 */


#include "pid.h"
#include "stdlib.h"
#include "math.h"
#include <stdio.h>


static PIDController pid;
static uint16_t base_speed=MAX_CCR - 100;
static uint8_t front_distance;
static uint8_t left45_distance;
static uint8_t right45_distance;
static uint16_t left_speed;
static uint16_t right_speed;

static uint8_t total_width;
static uint8_t idealPosition;
static uint8_t currentPosition;
static int16_t pid_output;
static int16_t correction;

void pid_init(PIDController *pid, float kp, float Ki, float Kd, uint16_t dt )
{
   pid->Kp = kp;
   pid->Ki = Ki;
   pid->Kd = Kd;
   pid->dt = dt;
   pid->integral = 0.0f;
   pid->prev_error = 0.0f;

}
void pid_module_init() {
    // 차동 구동에 최적화된 PID 상수
    // Kp: 빠른 응답을 위해 충분히 높게
    // Ki: 정상 상태 오차를 제거하기 위해 적당히
    // Kd: 오버슈트 방지와 안정화를 위해
    pid_init(&pid, 15.0f, 0.01f, 0.1f, 100);
}
uint16_t pid_compute(PIDController *pid, int8_t error)
{
      // 비례 항
      float p = pid->Kp * error;

      // 적분 항 (안티-와인드업 포함)
      pid->integral += error ; // dt를 초 단위로 변환
      float integral_max = MAX_CCR /(2.0f * pid->Ki);

      if (pid->integral > integral_max) pid->integral = integral_max;
      if (pid->integral < -integral_max) pid->integral = -integral_max;

      float i = pid->Ki * pid->integral;
//
//      // 미분 항 - 노이즈에 강건하게 필터링 적용
//      float derivative = (error - pid->prev_error) / pid->dt; // 초당 변화율
//      // 간단한 로우패스 필터 적용 (급격한 변화 완화)
//      float d = pid->Kd * derivative;

      // 미분 항에 저역통과 필터 적용
      static float prev_derivative = 0.0f;
      float derivative = (error - pid->prev_error) / (float)pid->dt;
      derivative = 0.8f * prev_derivative + 0.2f * derivative; // 간단한 LPF
      prev_derivative = derivative;
      float d = pid->Kd * derivative;

      // 출력 계산
      pid_output = p + i + d;
      printf("pid_output : %d",pid_output);

      // 이전 오차 업데이트
      pid->prev_error = error;
      if (pid_output > MAX_CCR) pid_output = MAX_CCR;
      if (pid_output < -MAX_CCR) pid_output = -MAX_CCR;
      return pid_output;
}

void pid_reset_integral() {
    pid.integral = 0.0f;
    pid.prev_error = 0.0f;
}

void pid_process()
{
  front_distance = *get_hcsr04_front_filtered_distance_ptr();
  left45_distance = *get_hcsr04_left90_filtered_distance_ptr() ;
  right45_distance = *get_hcsr04_right90_filtered_distance_ptr();

  // 디버깅: 센서 데이터 출력

  // 벽간 전체 폭 계산
  total_width = left45_distance + right45_distance + CAR_WIDTH;

  idealPosition = total_width / 2;
  currentPosition = left45_distance + (CAR_WIDTH / 2);
  int8_t error = currentPosition - idealPosition;

  correction = pid_compute(&pid, error);
  if (left45_distance < 5 || right45_distance < 5)
  {
      pid_reset_integral(); // 벽에 너무 가까우면 적분 항 리셋
  }
  // 모터 속도 조정
  left_speed = base_speed - correction;  // 왼쪽 치우침(error < 0) -> correction 양수 -> left_speed 증가
  right_speed = base_speed + correction; // 왼쪽 치우침(error < 0) -> correction 양수 -> right_speed 감소

  // 속도 제한
  if (left_speed > MAX_CCR) left_speed = MAX_CCR;
  if (left_speed < 0) left_speed = 0;
  if (right_speed > MAX_CCR) right_speed = MAX_CCR;
  if (right_speed < 0) right_speed = 0;



  motor_move(1, 1);
  motor_speed(right_speed, left_speed);

}

