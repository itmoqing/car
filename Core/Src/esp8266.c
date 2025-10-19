#include "esp8266.h"

// 全局变量定义
ESP8266_Config_t esp8266_config = {
    .wifi_ssid = "TT-",
    .wifi_password = "1822077154",
    .mqtt_client_id = "itmoqing1",
    .mqtt_username = "",
    .mqtt_password = "",
    .mqtt_server = "itmojun.com",
    .mqtt_port = 1883,
    .subscribe_topic = "itmoqing1/cmd",
    .publish_topic = "itmoqing1/status"
};

ESP8266_State_t esp8266_current_state = ESP_STATE_INIT;
UART_HandleTypeDef *esp8266_huart;
char mqtt_received_cmd = 's';  // 默认停止命令

// 接收缓冲区
uint8_t rx_buffer[512];
uint16_t rx_index = 0;

// 初始化ESP8266
void ESP8266_Init(UART_HandleTypeDef *huart)
{
    esp8266_huart = huart;
    esp8266_current_state = ESP_STATE_INIT;
    
    printf("[ESP8266] Initializing in listen mode...\r\n");
    
    // 清空接收缓冲区
    memset(rx_buffer, 0, sizeof(rx_buffer));
    rx_index = 0;
    
    // 启动串口接收中断
    HAL_UART_Receive_IT(esp8266_huart, &rx_buffer[rx_index], 1);
    printf("[ESP8266] UART receive interrupt started\r\n");
}

// 主函数：自动检查并连接 - 简化版本
ESP8266_Status_t ESP8266_Auto_CheckAndConnect(void)
{
    printf("[ESP8266] ===== Starting in Listen Only Mode =====\r\n");
    printf("[ESP8266] Assuming ESP8266 is already configured\r\n");
    printf("[ESP8266] WiFi: %s\r\n", esp8266_config.wifi_ssid);
    printf("[ESP8266] MQTT Server: %s\r\n", esp8266_config.mqtt_server);
    printf("[ESP8266] Subscribe Topic: %s\r\n", esp8266_config.subscribe_topic);
    printf("[ESP8266] Listening for MQTT messages...\r\n");
    
    // 直接设置为已订阅状态，开始监听
    esp8266_current_state = ESP_STATE_SUBSCRIBED;
    
    printf("[ESP8266] Ready to receive MQTT commands!\r\n");
    return ESP8266_OK;
}

// 检查模块是否就绪 - 简化版本
ESP8266_Status_t ESP8266_Check_Ready(void)
{
    printf("[ESP8266] Skip AT check, assume module is ready\r\n");
    return ESP8266_OK;
}

// 检查WiFi连接状态 - 简化版本
ESP8266_Status_t ESP8266_Check_WiFi(void)
{
    printf("[ESP8266] Skip WiFi check\r\n");
    return ESP8266_OK;
}

// 检查MQTT连接状态 - 简化版本
ESP8266_Status_t ESP8266_Check_MQTT(void)
{
    printf("[ESP8266] Skip MQTT check\r\n");
    return ESP8266_OK;
}

// 连接WiFi - 简化版本
ESP8266_Status_t ESP8266_Connect_WiFi(void)
{
    printf("[ESP8266] Skip WiFi connection\r\n");
    return ESP8266_OK;
}

// 连接MQTT - 简化版本
ESP8266_Status_t ESP8266_Connect_MQTT(void)
{
    printf("[ESP8266] Skip MQTT connection\r\n");
    return ESP8266_OK;
}

// 订阅主题 - 简化版本
ESP8266_Status_t ESP8266_Subscribe_Topic(void)
{
    printf("[ESP8266] Skip topic subscription\r\n");
    return ESP8266_OK;
}

// 发布消息 - 保留功能
ESP8266_Status_t ESP8266_Publish_Message(char *topic, char *message, uint8_t qos)
{
    // 由于UART发送有问题，暂时注释掉发布功能
    printf("[ESP8266] Publish disabled due to UART issues\r\n");
    return ESP8266_OK;
}

// 发送AT指令 - 保留但禁用
void ESP8266_Send_Command(char *cmd, uint32_t timeout)
{
    printf("[ESP8266] AT commands disabled: %s\r\n", cmd);
}

// 等待响应 - 简化版本
ESP8266_Status_t ESP8266_Wait_Response(char *expected_response, uint32_t timeout)
{
    printf("[ESP8266] Skip waiting for response\r\n");
    return ESP8266_OK;
}

// 解析MQTT消息
void ESP8266_Parse_MQTT_Message(char *message)
{
    printf("[ESP8266] Parsing MQTT message: %s\r\n", message);
    
    // 解析格式: +MQTTSUBRECV:0,"itmoqing1/cmd",1,g
    char *topic_start = strstr(message, "\"");
    if (topic_start) {
        char *topic_end = strstr(topic_start + 1, "\"");
        if (topic_end) {
            char *cmd_start = strstr(topic_end + 1, "\"");
            if (cmd_start) {
                char *cmd_end = strstr(cmd_start + 1, "\"");
                if (cmd_end) {
                    // 提取命令字符
                    mqtt_received_cmd = *(cmd_start + 1);
                    printf("[ESP8266] MQTT command received: %c\r\n", mqtt_received_cmd);
                    return;
                }
            }
        }
    }
    
    printf("[ESP8266] ERROR: Failed to parse MQTT message\r\n");
}

// 处理接收到的数据 - 带完整调试信息
void ESP8266_Process_Received_Data(uint8_t *data, uint16_t length)
{
    if (rx_index + length < sizeof(rx_buffer)) {
        memcpy(&rx_buffer[rx_index], data, length);
        rx_index += length;
        rx_buffer[rx_index] = '\0';
        
        // 显示所有接收到的原始数据
        if (length > 0) {
            printf("[UART_RX] Received %d bytes: ", length);
            for(int i = 0; i < length; i++) {
                if(data[i] >= 32 && data[i] <= 126) {
                    printf("%c", data[i]);
                } else {
                    printf("[0x%02X]", data[i]);
                }
            }
            printf("\r\n");
            
            // 显示当前缓冲区内容
            printf("[BUFFER] Current buffer (%d bytes): ", rx_index);
            for(int i = 0; i < rx_index; i++) {
                if(rx_buffer[i] >= 32 && rx_buffer[i] <= 126) {
                    printf("%c", rx_buffer[i]);
                } else {
                    printf("[0x%02X]", rx_buffer[i]);
                }
            }
            printf("\r\n");
        }
        
        // 检查是否是MQTT消息
        if (strstr((char*)rx_buffer, "+MQTTSUBRECV") != NULL) {
            printf("[ESP8266] Detected MQTT message in buffer\r\n");
            ESP8266_Parse_MQTT_Message((char*)rx_buffer);
            memset(rx_buffer, 0, sizeof(rx_buffer));
            rx_index = 0;
        }
        
        // 防止缓冲区溢出
        if (rx_index >= sizeof(rx_buffer) - 1) {
            printf("[ESP8266] WARNING: RX buffer overflow, clearing...\r\n");
            memset(rx_buffer, 0, sizeof(rx_buffer));
            rx_index = 0;
        }
    }
}

// ESP8266串口接收中断回调
void ESP8266_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == esp8266_huart) {
        ESP8266_Process_Received_Data(&rx_buffer[rx_index], 1);
        rx_index++;
        
        if (rx_index < sizeof(rx_buffer) - 1) {
            HAL_UART_Receive_IT(esp8266_huart, &rx_buffer[rx_index], 1);
        } else {
            printf("[ESP8266] RX buffer full, resetting...\r\n");
            rx_index = 0;
            HAL_UART_Receive_IT(esp8266_huart, &rx_buffer[rx_index], 1);
        }
    }
}
