#ifndef __TB6612FNG_H
#define __TB6612FNG_H

#include "stm32f1xx_hal.h"

// TB6612FNG引脚定义
// 电机A控制引脚
#define PWMA_PIN       GPIO_PIN_0
#define PWMA_PORT      GPIOA
#define AIN1_PIN       GPIO_PIN_1
#define AIN1_PORT      GPIOA
#define AIN2_PIN       GPIO_PIN_2
#define AIN2_PORT      GPIOA

// 电机B控制引脚
#define PWMB_PIN       GPIO_PIN_3
#define PWMB_PORT      GPIOA
#define BIN1_PIN       GPIO_PIN_4
#define BIN1_PORT      GPIOA
#define BIN2_PIN       GPIO_PIN_5
#define BIN2_PORT      GPIOA

// STBY引脚(待机控制)
#define STBY_PIN       GPIO_PIN_6
#define STBY_PORT      GPIOA

// 电机方向定义
#define FORWARD        1
#define BACKWARD       0

// 函数声明
void TB6612FNG_Init(void);
void TB6612FNG_Standby(void);
void TB6612FNG_Wakeup(void);
void MotorA_SetDirection(uint8_t dir);
void MotorB_SetDirection(uint8_t dir);
void MotorA_SetSpeed(uint16_t speed);
void MotorB_SetSpeed(uint16_t speed);
void MotorA_Stop(void);
void MotorB_Stop(void);
void Motors_Stop(void);

#endif /* __TB6612FNG_H */
