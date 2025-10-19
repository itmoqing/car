#ifndef __MOTOR_H
#define __MOTOR_H

#include "main.h"

// 电机1 (TIM3_CH1)
#define MOTOR1_PWM_PIN          GPIO_PIN_6
#define MOTOR1_PWM_PORT         GPIOA
#define MOTOR1_PWM_TIM          &htim3
#define MOTOR1_PWM_CHANNEL      TIM_CHANNEL_1
#define MOTOR1_DIR1_PIN         GPIO_PIN_0
#define MOTOR1_DIR2_PIN         GPIO_PIN_1
#define MOTOR1_DIR_PORT         GPIOB

// 电机2 (TIM3_CH2)
#define MOTOR2_PWM_PIN          GPIO_PIN_7
#define MOTOR2_PWM_PORT         GPIOA
#define MOTOR2_PWM_TIM          &htim3
#define MOTOR2_PWM_CHANNEL      TIM_CHANNEL_2
#define MOTOR2_DIR1_PIN         GPIO_PIN_11      // ✅ 改为 PA11
#define MOTOR2_DIR2_PIN         GPIO_PIN_12      // ✅ 改为 PA12
#define MOTOR2_DIR_PORT         GPIOA            // ✅ PA11/PA12 控制方向

// 电机3 (TIM4_CH1)
#define MOTOR3_PWM_PIN          GPIO_PIN_6
#define MOTOR3_PWM_PORT         GPIOB
#define MOTOR3_PWM_TIM          &htim4
#define MOTOR3_PWM_CHANNEL      TIM_CHANNEL_1
#define MOTOR3_DIR1_PIN         GPIO_PIN_4
#define MOTOR3_DIR2_PIN         GPIO_PIN_5
#define MOTOR3_DIR_PORT         GPIOB

// 电机4 (TIM4_CH2)
#define MOTOR4_PWM_PIN          GPIO_PIN_7
#define MOTOR4_PWM_PORT         GPIOB
#define MOTOR4_PWM_TIM          &htim4
#define MOTOR4_PWM_CHANNEL      TIM_CHANNEL_2
#define MOTOR4_DIR1_PIN         GPIO_PIN_8
#define MOTOR4_DIR2_PIN         GPIO_PIN_9
#define MOTOR4_DIR_PORT         GPIOB

// PWM 范围定义
#define PWM_MAX                 7200
#define PWM_MIN                -7200

// 函数声明
void MOTOR_Init(void);
void Set_Motor_PWM(int motor1, int motor2, int motor3, int motor4);
void Set_Single_Motor(int motor_num, int pwm_value);
void Car_Forward(int speed);
void Car_Backward(int speed);
void Car_TurnLeft(int speed);
void Car_TurnRight(int speed);
void Car_Stop(void);

#endif
