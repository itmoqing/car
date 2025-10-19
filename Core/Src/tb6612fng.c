#include "tb6612fng.h"

// PWM配置参数
TIM_HandleTypeDef htim_pwm;

/**
 * @brief  TB6612FNG初始化函数
 * @param  无
 * @retval 无
 */
void TB6612FNG_Init(void)
{
    // 1. 初始化GPIO引脚
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // 使能GPIO时钟
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    // 配置控制引脚为输出模式
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    
    // 电机A控制引脚
    GPIO_InitStruct.Pin = AIN1_PIN | AIN2_PIN;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // 电机B控制引脚
    GPIO_InitStruct.Pin = BIN1_PIN | BIN2_PIN;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // STBY引脚
    GPIO_InitStruct.Pin = STBY_PIN;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // 初始化PWM引脚
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    
    // PWMA引脚
    GPIO_InitStruct.Pin = PWMA_PIN;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // PWMB引脚
    GPIO_InitStruct.Pin = PWMB_PIN;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // 2. 初始化PWM定时器
    __HAL_RCC_TIM2_CLK_ENABLE();
    
    htim_pwm.Instance = TIM2;
    htim_pwm.Init.Prescaler = 71;  // 假设系统时钟为72MHz，分频后为1MHz
    htim_pwm.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim_pwm.Init.Period = 999;    // PWM周期为1ms (1kHz)
    htim_pwm.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim_pwm.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_PWM_Init(&htim_pwm);
    
    // 配置PWM通道
    TIM_OC_InitTypeDef sConfigOC = {0};
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;           // 初始占空比为0
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    
    // 配置TIM2通道1 (PWMA)
    HAL_TIM_PWM_ConfigChannel(&htim_pwm, &sConfigOC, TIM_CHANNEL_1);
    
    // 配置TIM2通道2 (PWMB)
    HAL_TIM_PWM_ConfigChannel(&htim_pwm, &sConfigOC, TIM_CHANNEL_2);
    
    // 启动PWM
    HAL_TIM_PWM_Start(&htim_pwm, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim_pwm, TIM_CHANNEL_2);
    
    // 3. 唤醒TB6612FNG（退出待机模式）
    TB6612FNG_Wakeup();
    
    // 4. 初始状态下停止所有电机
    Motors_Stop();
}

/**
 * @brief  设置TB6612FNG进入待机模式
 * @param  无
 * @retval 无
 */
void TB6612FNG_Standby(void)
{
    HAL_GPIO_WritePin(STBY_PORT, STBY_PIN, GPIO_PIN_RESET);
}

/**
 * @brief  唤醒TB6612FNG（退出待机模式）
 * @param  无
 * @retval 无
 */
void TB6612FNG_Wakeup(void)
{
    HAL_GPIO_WritePin(STBY_PORT, STBY_PIN, GPIO_PIN_SET);
}

/**
 * @brief  设置电机A的旋转方向
 * @param  dir: 方向 (FORWARD或BACKWARD)
 * @retval 无
 */
void MotorA_SetDirection(uint8_t dir)
{
    if(dir == FORWARD)
    {
        HAL_GPIO_WritePin(AIN1_PORT, AIN1_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(AIN2_PORT, AIN2_PIN, GPIO_PIN_RESET);
    }
    else
    {
        HAL_GPIO_WritePin(AIN1_PORT, AIN1_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(AIN2_PORT, AIN2_PIN, GPIO_PIN_SET);
    }
}

/**
 * @brief  设置电机B的旋转方向
 * @param  dir: 方向 (FORWARD或BACKWARD)
 * @retval 无
 */
void MotorB_SetDirection(uint8_t dir)
{
    if(dir == FORWARD)
    {
        HAL_GPIO_WritePin(BIN1_PORT, BIN1_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(BIN2_PORT, BIN2_PIN, GPIO_PIN_RESET);
    }
    else
    {
        HAL_GPIO_WritePin(BIN1_PORT, BIN1_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(BIN2_PORT, BIN2_PIN, GPIO_PIN_SET);
    }
}

/**
 * @brief  设置电机A的速度
 * @param  speed: 速度值 (0-999)
 * @retval 无
 */
void MotorA_SetSpeed(uint16_t speed)
{
    // 确保速度值在有效范围内
    if(speed > 999) speed = 999;
    
    // 设置PWM占空比
    __HAL_TIM_SET_COMPARE(&htim_pwm, TIM_CHANNEL_1, speed);
}

/**
 * @brief  设置电机B的速度
 * @param  speed: 速度值 (0-999)
 * @retval 无
 */
void MotorB_SetSpeed(uint16_t speed)
{
    // 确保速度值在有效范围内
    if(speed > 999) speed = 999;
    
    // 设置PWM占空比
    __HAL_TIM_SET_COMPARE(&htim_pwm, TIM_CHANNEL_2, speed);
}

/**
 * @brief  停止电机A
 * @param  无
 * @retval 无
 */
void MotorA_Stop(void)
{
    HAL_GPIO_WritePin(AIN1_PORT, AIN1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(AIN2_PORT, AIN2_PIN, GPIO_PIN_RESET);
    MotorA_SetSpeed(0);
}

/**
 * @brief  停止电机B
 * @param  无
 * @retval 无
 */
void MotorB_Stop(void)
{
    HAL_GPIO_WritePin(BIN1_PORT, BIN1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(BIN2_PORT, BIN2_PIN, GPIO_PIN_RESET);
    MotorB_SetSpeed(0);
}

/**
 * @brief  停止所有电机
 * @param  无
 * @retval 无
 */
void Motors_Stop(void)
{
    MotorA_Stop();
    MotorB_Stop();
}