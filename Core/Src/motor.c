#include "motor.h"
#include "tim.h"

// 电机初始化
void MOTOR_Init(void)
{
    // 启动所有PWM通道
    HAL_TIM_PWM_Start(MOTOR1_PWM_TIM, MOTOR1_PWM_CHANNEL);
    HAL_TIM_PWM_Start(MOTOR2_PWM_TIM, MOTOR2_PWM_CHANNEL);
    HAL_TIM_PWM_Start(MOTOR3_PWM_TIM, MOTOR3_PWM_CHANNEL);
    HAL_TIM_PWM_Start(MOTOR4_PWM_TIM, MOTOR4_PWM_CHANNEL);
    
    // 初始化所有电机为停止状态
    Car_Stop();
}

// 设置单个电机
void Set_Single_Motor(int motor_num, int pwm_value)
{
    // 限制PWM值在有效范围内
    if(pwm_value > PWM_MAX) pwm_value = PWM_MAX;
    if(pwm_value < PWM_MIN) pwm_value = PWM_MIN;
    
    GPIO_TypeDef* dir_port;
    uint16_t dir1_pin, dir2_pin;
    TIM_HandleTypeDef* pwm_tim;
    uint32_t pwm_channel;
    
    // 根据电机编号选择对应的引脚和定时器
    switch(motor_num)
    {
        case 1:
            dir_port = MOTOR1_DIR_PORT;
            dir1_pin = MOTOR1_DIR1_PIN;
            dir2_pin = MOTOR1_DIR2_PIN;
            pwm_tim = MOTOR1_PWM_TIM;
            pwm_channel = MOTOR1_PWM_CHANNEL;
            break;
            
        case 2:
            dir_port = MOTOR2_DIR_PORT;    
            dir1_pin = MOTOR2_DIR1_PIN;    
            dir2_pin = MOTOR2_DIR2_PIN;    
            pwm_tim = MOTOR2_PWM_TIM;
            pwm_channel = MOTOR2_PWM_CHANNEL;
            break;
            
        case 3:
            dir_port = MOTOR3_DIR_PORT;
            dir1_pin = MOTOR3_DIR1_PIN;
            dir2_pin = MOTOR3_DIR2_PIN;
            pwm_tim = MOTOR3_PWM_TIM;
            pwm_channel = MOTOR3_PWM_CHANNEL;
            break;
            
        case 4:
            dir_port = MOTOR4_DIR_PORT;
            dir1_pin = MOTOR4_DIR1_PIN;
            dir2_pin = MOTOR4_DIR2_PIN;
            pwm_tim = MOTOR4_PWM_TIM;
            pwm_channel = MOTOR4_PWM_CHANNEL;
            break;
            
        default:
            return; // 无效的电机编号
    }
    
    // 控制电机方向和速度
    if(pwm_value > 0) 
    {
        // 正转
        __HAL_TIM_SET_COMPARE(pwm_tim, pwm_channel, pwm_value);
        HAL_GPIO_WritePin(dir_port, dir1_pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(dir_port, dir2_pin, GPIO_PIN_RESET);
    } 
    else if(pwm_value < 0) 
    {
        // 反转
        __HAL_TIM_SET_COMPARE(pwm_tim, pwm_channel, -pwm_value);
        HAL_GPIO_WritePin(dir_port, dir1_pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(dir_port, dir2_pin, GPIO_PIN_SET);
    } 
    else 
    {
        // 停止
        __HAL_TIM_SET_COMPARE(pwm_tim, pwm_channel, 0);
        HAL_GPIO_WritePin(dir_port, dir1_pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(dir_port, dir2_pin, GPIO_PIN_RESET);
    }
}

// 设置所有4个电机
void Set_Motor_PWM(int motor1, int motor2, int motor3, int motor4)
{
    Set_Single_Motor(1, motor1);
    Set_Single_Motor(2, motor2);
    Set_Single_Motor(3, motor3);
    Set_Single_Motor(4, motor4);
}

// 小车运动控制函数
void Car_Forward(int speed)
{
    Set_Motor_PWM(speed, speed, speed, speed);
}

void Car_Backward(int speed)
{
    Set_Motor_PWM(-speed, -speed, -speed, -speed);
}

void Car_TurnLeft(int speed)
{
    Set_Motor_PWM(-speed, speed, -speed, speed);
}

void Car_TurnRight(int speed)
{
    Set_Motor_PWM(speed, -speed, speed, -speed);
}

void Car_Stop(void)
{
    Set_Motor_PWM(0, 0, 0, 0);
}
