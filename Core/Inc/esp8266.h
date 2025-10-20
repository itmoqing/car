#ifndef INC_ESP8266_H_
#define INC_ESP8266_H_

#include "main.h" // 包含HAL库和其他基础定义
#include "usart.h" // 包含USART句柄定义
#include <stdbool.h> // 用于 bool 类型

//===================== 配置结构体 =====================//
typedef struct {
    char *wifi_ssid;
    char *wifi_password;
    char *mqtt_client_id;
    char *mqtt_username;
    char *mqtt_password;
    char *mqtt_server;
    uint16_t mqtt_port;
    
    // 新增：保存DEVICE_ID，用于构建主题
    char *device_id; 

} ESP8266_Config_t;

//===================== 状态枚举 =====================//
typedef enum {
    ESP_STATE_INIT,
    ESP_STATE_READY,
    ESP_STATE_WIFI_CONNECTED,
    ESP_STATE_CONNECTED, // MQTT Connected
    ESP_STATE_SUBSCRIBED,
    ESP_STATE_ERROR
} ESP8266_State_t;

//===================== 状态返回枚举 =====================//
typedef enum {
    ESP8266_OK = 0,
    ESP8266_ERROR
} ESP8266_Status_t;

//===================== 全局变量声明 =====================//
extern ESP8266_Config_t esp8266_config; // 外部声明配置结构体
extern ESP8266_State_t esp8266_current_state;
extern UART_HandleTypeDef *esp8266_huart;
extern char mqtt_received_cmd; // 供 main.c 访问的 MQTT 命令
extern uint8_t esp8266_rx_byte; // 单字节临时接收缓冲区，供 main.c 的回调使用

//===================== 函数声明 =====================//

void ESP8266_Init(UART_HandleTypeDef *huart);
ESP8266_Status_t ESP8266_Auto_CheckAndConnect(void);
ESP8266_Status_t ESP8266_Publish_Message(char *message, uint8_t qos); // 修改：不再需要topic参数，使用内置topic

// 此函数由 main.c 中的 HAL_UART_RxCpltCallback 调用
void ESP8266_Process_Received_Byte(uint8_t received_byte);


#endif /* INC_ESP8266_H_ */
