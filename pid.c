/*
 * pid.c
 *
 *  Created on: May 19, 2025
 *      Author: USER
 */


#include "pid.h"
#include "stdlib.h"
#include "math.h"


static PIDController pid;
static uint16_t base_speed=MAX_CCR;
static uint8_t front_distance;
static uint8_t left90_distance;
static uint8_t right90_distance;
static uint16_t left_speed;
static uint16_t right_speed;

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
    pid_init(&pid, 80.0f, 0.2f, 15.0f, 10);
}
uint16_t pid_compute(PIDController *pid, uint16_t error)
{
      // 비례 항
      float p = pid->Kp * error;

      // 적분 항 (안티-와인드업 포함)
      pid->integral += error * pid->dt / 1000.0f; // dt를 초 단위로 변환
      float integral_max = MAX_CCR /(2.0f * pid->Ki);

      if (pid->integral > integral_max) pid->integral = integral_max;
      if (pid->integral < -integral_max) pid->integral = -integral_max;

      float i = pid->Ki * pid->integral;

      // 미분 항 - 노이즈에 강건하게 필터링 적용
      float derivative = (error - pid->prev_error) * 1000.0f / pid->dt; // 초당 변화율
      // 간단한 로우패스 필터 적용 (급격한 변화 완화)
      static float filtered_derivative = 0.0f;
      filtered_derivative = 0.7f * filtered_derivative + 0.3f * derivative;

      float d = pid->Kd * filtered_derivative;
      // 출력 계산
      float output = p + i + d;

      // 이전 오차 업데이트
      pid->prev_error = error;



      return output;  // 절대값 반환, 방향은 pid_process에서 처리
}

void pid_reset_integral() {
    pid.integral = 0.0f;
    pid.prev_error = 0.0f;
}

void pid_process()
{
  front_distance = *get_hcsr04_front_filtered_distance_ptr();
  left90_distance = *get_hcsr04_left90_filtered_distance_ptr();
  right90_distance = *get_hcsr04_right90_filtered_distance_ptr();

  // 디버깅: 센서 데이터 출력
  printf("Left: %d cm, Right: %d cm, Front: %d cm\n",
            left90_distance, right90_distance, front_distance);
  // 전방 장애물 처리
  if (front_distance <= MIN_FRONT_DISTANCE) {
      base_speed = MIN_CCR;
      motor_move(0, 0); // 정지
      pid_reset_integral();
      return;
  }
  else if (front_distance < FRONT_THRESHOLD)
  {
      base_speed = MIN_CCR + (MAX_CCR - MIN_CCR) *
		    (front_distance - MIN_FRONT_DISTANCE) /
		    (FRONT_THRESHOLD - MIN_FRONT_DISTANCE);
  }
  else
  {
      base_speed = MAX_CCR;
  }

  // 벽 충돌 예방을 위한 긴급 회피 처리
  if (left90_distance < MIN_WALL_DISTANCE)
  {
      motor_move(1, 1);
      motor_speed(MIN_CCR, MAX_CCR);
      pid_reset_integral(); // 적분항 초기화
      return;
  }
  else if (right90_distance < MIN_WALL_DISTANCE)
  {
      motor_move(1, 1);
      motor_speed(MIN_CCR, MAX_CCR);
      pid_reset_integral(); // 적분항 초기화
      return;
  }



  // 오차 계산: 왼쪽 벽 가까우면 error > 0
  float error = 0.0f;
  if (left90_distance < 100 && right90_distance < 100)
  {
      error = (float)(right90_distance - left90_distance) ; // error > 0: 왼쪽 벽 가까움
  }
  else if (left90_distance < 100)
  {
      error = (50.0f - left90_distance); // 왼쪽 벽 기준, 목표 50cm
  }
  else if (right90_distance < 100)
  {
      error = (right90_distance - 50.0f); // 오른쪽 벽 기준
  }

   // PID 제어 출력 계산
   float steering_output = pid_compute(&pid, error);

   // steering_output 스케일링 (MAX_CCR의 30% 이내)
   float max_steering = MAX_CCR * 0.25f;
   if (steering_output > max_steering) steering_output = max_steering;
   if (steering_output < -max_steering) steering_output = -max_steering;

   // 속도 계산: error > 0 (왼쪽 벽 가까움) -> 왼쪽 모터 빠르게
    left_speed = base_speed + steering_output;
    right_speed = base_speed - steering_output;

    // 속도 제한 (부드러운 제어를 위해 최소 제한 완화)
    if (left_speed > MAX_CCR) left_speed = MAX_CCR;
    if (left_speed < 0) left_speed = 0; // 음수 속도 방지
    if (right_speed > MAX_CCR) right_speed = MAX_CCR;
    if (right_speed < 0) right_speed = 0;

    // 디버깅 출력
    // 디버깅 출력
      printf("Error: %.2f, Steering: %.2f, Left: %d, Right: %d, Base: %d\n",
             error, steering_output, left_speed, right_speed, base_speed);
   // 모터 제어
   motor_move(1, 1); // 전진
   motor_speed(left_speed, right_speed);
}

