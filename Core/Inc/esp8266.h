#ifndef __ESP8266_H
#define __ESP8266_H

#include "main.h"
#include <string.h>
#include <stdio.h>

// ESP8266状态定义
typedef enum {
    ESP8266_OK = 0,
    ESP8266_ERROR,
    ESP8266_TIMEOUT,
    ESP8266_NOT_READY,
    ESP8266_WIFI_DISCONNECTED,
    ESP8266_MQTT_DISCONNECTED
} ESP8266_Status_t;

// ESP8266连接状态
typedef enum {
    ESP_STATE_INIT = 0,
    ESP_STATE_READY,
    ESP_STATE_WIFI_CONNECTED,
    ESP_STATE_MQTT_CONNECTED,
    ESP_STATE_SUBSCRIBED,
    ESP_STATE_ERROR
} ESP8266_State_t;

// 配置参数结构体
typedef struct {
    char wifi_ssid[32];
    char wifi_password[64];
    char mqtt_client_id[32];
    char mqtt_username[32];
    char mqtt_password[32];
    char mqtt_server[64];
    uint16_t mqtt_port;
    char subscribe_topic[64];
    char publish_topic[64];
} ESP8266_Config_t;

// 外部变量声明
extern ESP8266_Config_t esp8266_config;
extern ESP8266_State_t esp8266_current_state;
extern char mqtt_received_cmd;
extern uint8_t rx_buffer[512];
extern uint16_t rx_index;

// 函数声明
void ESP8266_Init(UART_HandleTypeDef *huart);
ESP8266_Status_t ESP8266_Auto_CheckAndConnect(void);
ESP8266_Status_t ESP8266_Check_Ready(void);
ESP8266_Status_t ESP8266_Check_WiFi(void);
ESP8266_Status_t ESP8266_Check_MQTT(void);
ESP8266_Status_t ESP8266_Connect_WiFi(void);
ESP8266_Status_t ESP8266_Connect_MQTT(void);
ESP8266_Status_t ESP8266_Subscribe_Topic(void);
ESP8266_Status_t ESP8266_Publish_Message(char *topic, char *message, uint8_t qos);
void ESP8266_Send_Command(char *cmd, uint32_t timeout);
ESP8266_Status_t ESP8266_Wait_Response(char *expected_response, uint32_t timeout);
void ESP8266_Process_Received_Data(uint8_t *data, uint16_t length);
void ESP8266_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void ESP8266_Parse_MQTT_Message(char *message);

#endif
