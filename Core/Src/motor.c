#include "motor.h"
#include "tim.h"

// 转向参数配置 - 低电量大半径转向优化
#define INNER_WHEEL_FACTOR 0.2f    // 内侧轮子低速转动（保持控制）
#define OUTER_WHEEL_FACTOR 0.6f    // 外侧轮子中速转动（省电）
#define TURN_RADIUS_FACTOR 0.8f    // 转向半径系数

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

// 设置单个电机（保持不变）
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

// 设置所有4个电机（保持不变）
void Set_Motor_PWM(int motor1, int motor2, int motor3, int motor4)
{
    Set_Single_Motor(1, motor1);
    Set_Single_Motor(2, motor2);
    Set_Single_Motor(3, motor3);
    Set_Single_Motor(4, motor4);
}

// 小车运动控制函数 - 低电量优化版本
void Car_Forward(int speed)
{
    // 低电量时降低前进速度
    int low_power_speed = speed * 0.7f;
    Set_Motor_PWM(low_power_speed, low_power_speed, low_power_speed, low_power_speed);
}

void Car_Backward(int speed)
{
    // 低电量时降低后退速度
    int low_power_speed = speed * 0.7f;
    Set_Motor_PWM(-low_power_speed, -low_power_speed, -low_power_speed, -low_power_speed);
}

// 大半径丝滑左转 - 低电量优化
void Car_TurnLeft(int speed)
{
    // 计算内外侧轮子速度（低速省电）
    int inner_speed = (int)(speed * INNER_WHEEL_FACTOR);  // 内侧轮子低速
    int outer_speed = (int)(speed * OUTER_WHEEL_FACTOR);  // 外侧轮子中速
    
    // 左转时：左轮为内侧，右轮为外侧
    // 大半径转向，内外侧速度差较小
    Set_Motor_PWM(inner_speed, outer_speed, inner_speed, outer_speed);
}

// 大半径丝滑右转 - 低电量优化
void Car_TurnRight(int speed)
{
    // 计算内外侧轮子速度（低速省电）
    int inner_speed = (int)(speed * INNER_WHEEL_FACTOR);  // 内侧轮子低速
    int outer_speed = (int)(speed * OUTER_WHEEL_FACTOR);  // 外侧轮子中速
    
    // 右转时：右轮为内侧，左轮为外侧
    // 大半径转向，内外侧速度差较小
    Set_Motor_PWM(outer_speed, inner_speed, outer_speed, inner_speed);
}

// 渐进式转向 - 更丝滑的大半径转向
void Car_GradualTurnLeft(int speed, uint8_t turn_stage)
{
    static const float stage_factors[3][2] = {
        {0.4f, 0.5f},  // 阶段1：小角度转向
        {0.2f, 0.6f},  // 阶段2：中角度转向  
        {0.1f, 0.7f}   // 阶段3：大角度转向
    };
    
    if(turn_stage > 2) turn_stage = 2;
    
    int inner_speed = (int)(speed * stage_factors[turn_stage][0]);
    int outer_speed = (int)(speed * stage_factors[turn_stage][1]);
    
    Set_Motor_PWM(inner_speed, outer_speed, inner_speed, outer_speed);
}

void Car_GradualTurnRight(int speed, uint8_t turn_stage)
{
    static const float stage_factors[3][2] = {
        {0.4f, 0.5f},  // 阶段1：小角度转向
        {0.2f, 0.6f},  // 阶段2：中角度转向  
        {0.1f, 0.7f}   // 阶段3：大角度转向
    };
    
    if(turn_stage > 2) turn_stage = 2;
    
    int inner_speed = (int)(speed * stage_factors[turn_stage][0]);
    int outer_speed = (int)(speed * stage_factors[turn_stage][1]);
    
    Set_Motor_PWM(outer_speed, inner_speed, outer_speed, inner_speed);
}

// 超低电量转向模式（两个轮子不转）
void Car_LowPowerTurnLeft(int speed)
{
    // 只有外侧两个轮子转动，内侧两个轮子不转
    // 形成大半径弧线转向
    Set_Motor_PWM(0, speed, 0, speed);
}

void Car_LowPowerTurnRight(int speed)
{
    // 只有外侧两个轮子转动，内侧两个轮子不转  
    // 形成大半径弧线转向
    Set_Motor_PWM(speed, 0, speed, 0);
}

// 原地转向（低电量时不推荐使用）
void Car_SpinLeft(int speed)
{
    // 低电量时降低原地转向速度
    int low_power_speed = speed * 0.5f;
    Set_Motor_PWM(-low_power_speed, low_power_speed, -low_power_speed, low_power_speed);
}

void Car_SpinRight(int speed)
{
    // 低电量时降低原地转向速度
    int low_power_speed = speed * 0.5f;
    Set_Motor_PWM(low_power_speed, -low_power_speed, low_power_speed, -low_power_speed);
}

void Car_Stop(void)
{
    Set_Motor_PWM(0, 0, 0, 0);
}
