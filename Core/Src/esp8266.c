#include "esp8266.h"
#include <string.h> // for strstr, memset, memcpy
#include <stdio.h>  // for snprintf

//===================== 全局配置 =====================//
// 注意：device_id 初始值在 main.c 中会被覆盖
ESP8266_Config_t esp8266_config = {
    .wifi_ssid = "TT-", // 你的WiFi SSID
    .wifi_password = "1822077154", // 你的WiFi密码
    .mqtt_client_id = "itmoqing1",
    .mqtt_username = "",
    .mqtt_password = "",
    .mqtt_server = "itmojun.com", // 你的MQTT服务器地址
    .mqtt_port = 1883,
    .device_id = "default_id" // 初始默认值，在 main.c 中会被实际 DEVICE_ID 宏覆盖
};

ESP8266_State_t esp8266_current_state = ESP_STATE_INIT;
UART_HandleTypeDef *esp8266_huart; // ESP8266使用的UART句柄
char mqtt_received_cmd = 's';      // 默认命令，供 main.c 读取

// 内部接收缓冲区，用于积累来自ESP8266的数据
static uint8_t internal_rx_buffer[512];
static uint16_t internal_rx_index = 0;

// 单字节临时接收缓冲区，由HAL库中断填充
uint8_t esp8266_rx_byte;

// 用于 ESP8266_Wait_Response 的状态标志和期待的响应字符串
static volatile bool response_received_flag = false;
static char *expected_response_str = NULL; // 期待的响应字符串，例如"OK", "ready"

// 动态构建订阅和发布主题的缓冲区
static char subscribe_topic_buffer[64];
static char publish_topic_buffer[64];

//===================== 工具函数 =====================//

/**
  * @brief  向ESP8266发送AT命令，并打印到调试串口。
  * @param  cmd: 要发送的命令字符串（不含CRLF）。
  * @param  timeout: 串口发送超时时间。
  * @retval None
  */
void ESP8266_Send_Command(char *cmd, uint32_t timeout)
{
    char command_with_crlf[256];
    snprintf(command_with_crlf, sizeof(command_with_crlf), "%s\r\n", cmd);
    HAL_UART_Transmit(esp8266_huart, (uint8_t *)command_with_crlf, strlen(command_with_crlf), timeout);
    printf("[ESP8266->] %s", command_with_crlf); // 打印到调试串口

    // ★ 每条命令后延时，防止ESP8266命令黏连
    HAL_Delay(50); // 适当减少延时，具体值可能需要根据实际情况调整
}

/**
  * @brief  等待ESP8266的特定响应。此函数为非阻塞式，通过检查中断接收设置的标志。
  * @param  expected: 期望收到的响应字符串，例如 "OK", "ready"。
  * @param  timeout: 等待响应的超时时间（毫秒）。
  * @retval ESP8266_OK: 收到期望响应。
  * @retval ESP8266_ERROR: 超时未收到。
  */
ESP8266_Status_t ESP8266_Wait_Response(char *expected, uint32_t timeout)
{
    uint32_t start_tick = HAL_GetTick();
    response_received_flag = false; // 清除上次的标志
    expected_response_str = expected; // 设置当前期望的响应

    // 清空缓冲区，确保只等待当前命令的响应
    memset(internal_rx_buffer, 0, sizeof(internal_rx_buffer));
    internal_rx_index = 0;

    printf("[ESP8266] Waiting for '%s'...\r\n", expected);

    while ((HAL_GetTick() - start_tick) < timeout) {
        if (response_received_flag) {
            response_received_flag = false; // 重置标志
            expected_response_str = NULL;   // 清除期待的响应
            printf("[ESP8266] Received expected response: '%s'\r\n", expected);
            return ESP8266_OK;
        }
        // 短暂延时，避免CPU空转，并允许中断处理
        HAL_Delay(10);
    }
    printf("[ESP8266] Timeout waiting for '%s'. Current buffer: %s\r\n", expected, (char*)internal_rx_buffer);
    expected_response_str = NULL;
    return ESP8266_ERROR;
}

//===================== 初始化 =====================//

/**
  * @brief  初始化ESP8266模块。
  * @param  huart: 用于与ESP8266通信的UART句柄。
  * @retval None
  */
void ESP8266_Init(UART_HandleTypeDef *huart)
{
    esp8266_huart = huart; // 绑定UART句柄
    esp8266_current_state = ESP_STATE_INIT;

    memset(internal_rx_buffer, 0, sizeof(internal_rx_buffer));
    internal_rx_index = 0;

    // 动态构建订阅和发布主题
    // 确保 esp8266_config.device_id 在调用 ESP8266_Init 之前已设置
    snprintf(subscribe_topic_buffer, sizeof(subscribe_topic_buffer), "%s/cmd", esp8266_config.device_id);
    snprintf(publish_topic_buffer, sizeof(publish_topic_buffer), "%s/status", esp8266_config.device_id);

    printf("[ESP8266] Initializing module...\r\n");
    // 启动单字节接收中断，接收到的字节将存入 esp8266_rx_byte
    HAL_UART_Receive_IT(esp8266_huart, &esp8266_rx_byte, 1);
}

//===================== MQTT连接流程 =====================//

/**
  * @brief  配置MQTT用户参数并连接到MQTT服务器。
  * @retval ESP8266_OK: 连接成功。
  * @retval ESP8266_ERROR: 连接失败。
  */
ESP8266_Status_t ESP8266_Connect_MQTT(void)
{
    char cmd[256];

    printf("[ESP8266] Configuring MQTT user parameters...\r\n");
    snprintf(cmd, sizeof(cmd),
             "AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"",
             esp8266_config.mqtt_client_id,
             esp8266_config.mqtt_username,
             esp8266_config.mqtt_password);
    ESP8266_Send_Command(cmd, 100);
    if (ESP8266_Wait_Response("OK", 6000) != ESP8266_OK) return ESP8266_ERROR;

    printf("[ESP8266] Connecting to MQTT server...\r\n");
    snprintf(cmd, sizeof(cmd),
             "AT+MQTTCONN=0,\"%s\",%d,0",
             esp8266_config.mqtt_server,
             esp8266_config.mqtt_port);
    ESP8266_Send_Command(cmd, 100);
    if (ESP8266_Wait_Response("OK", 8000) != ESP8266_OK) return ESP8266_ERROR;

    esp8266_current_state = ESP_STATE_CONNECTED;
    printf("[ESP8266] MQTT Connected Successfully!\r\n");
    return ESP8266_OK;
}

/**
  * @brief  订阅MQTT主题。
  * @retval ESP8266_OK: 订阅成功。
  * @retval ESP8266_ERROR: 订阅失败。
  */
ESP8266_Status_t ESP8266_Subscribe_Topic(void)
{
    char cmd[256];
    printf("[ESP8266] Subscribing to topic: %s\r\n", subscribe_topic_buffer);
    snprintf(cmd, sizeof(cmd), "AT+MQTTSUB=0,\"%s\",1", subscribe_topic_buffer);
    ESP8266_Send_Command(cmd, 100);
    if (ESP8266_Wait_Response("OK", 4000) != ESP8266_OK) return ESP8266_ERROR;

    esp8266_current_state = ESP_STATE_SUBSCRIBED;
    printf("[ESP8266] Subscribed successfully!\r\r\n"); // 额外CRLF
    return ESP8266_OK;
}

/**
  * @brief  发布MQTT消息。
  * @param  message: 消息内容。
  * @param  qos: QoS等级。
  * @retval ESP8266_OK: 发布成功。
  * @retval ESP8266_ERROR: 发布失败。
  */
ESP8266_Status_t ESP8266_Publish_Message(char *message, uint8_t qos)
{
    char cmd[256];
    printf("[ESP8266] Publishing to topic %s : %s\r\n", publish_topic_buffer, message);
    snprintf(cmd, sizeof(cmd), "AT+MQTTPUB=0,\"%s\",\"%s\",%d,0", publish_topic_buffer, message, qos);
    ESP8266_Send_Command(cmd, 100);
    if (ESP8266_Wait_Response("OK", 4000) != ESP8266_OK) return ESP8266_ERROR;
    return ESP8266_OK;
}

//===================== MQTT消息解析 =====================//

/**
  * @brief  解析接收到的MQTT消息，提取控制命令。
  * @param  message: 完整的MQTT消息字符串。
  * @retval None
  */
void ESP8266_Parse_MQTT_Message(char *message)
{
    printf("[ESP8266] Parsing MQTT message: %s\r\n", message);

    // 示例消息格式: +MQTTSUBRECV:0,"topic",payload_len,"payload"
    // 我们需要找到 payload
    char *payload_q_start = strrchr(message, '\"'); // 查找最后一个双引号
    if (payload_q_start && *(payload_q_start + 1) != '\0') {
        mqtt_received_cmd = *(payload_q_start + 1); // payload通常是单个字符命令
        printf("[ESP8266] MQTT Command Received: %c\r\n", mqtt_received_cmd);
    } else {
        printf("[ESP8266] MQTT Message parse error. Could not extract payload.\r\n");
    }
}

//===================== 串口接收中断处理 =====================//

/**
  * @brief  处理ESP8266 UART接收到的单个字节数据。
  *         此函数由 main.c 的 HAL_UART_RxCpltCallback 调用。
  * @param  received_byte: 接收到的一个字节。
  * @retval None
  */
void ESP8266_Process_Received_Byte(uint8_t received_byte)
{
    // 如果缓冲区未满，则将字节存入
    if (internal_rx_index < sizeof(internal_rx_buffer) - 1) {
        internal_rx_buffer[internal_rx_index++] = received_byte;
        internal_rx_buffer[internal_rx_index] = '\0'; // 始终保持字符串终止符

        // 检查是否收到了当前期望的响应 (例如 "OK", "ready", "WIFI GOT IP")
        if (expected_response_str != NULL && strstr((char*)internal_rx_buffer, expected_response_str) != NULL) {
            response_received_flag = true;
            // Note: 缓冲区不在此处清空，由 ESP8266_Wait_Response 清空以便打印完整响应
        }
        // 检查是否收到了 MQTT 消息接收指示 (+MQTTSUBRECV:)
        else if (strstr((char*)internal_rx_buffer, "+MQTTSUBRECV:") != NULL) {
            // MQTT消息可能跨多个中断，这里需要更智能的判断消息完整性
            // 简单判断：如果收到消息结束的 "\r\n"，则认为消息完整
            if (strstr((char*)internal_rx_buffer, "\r\n") != NULL) {
                ESP8266_Parse_MQTT_Message((char*)internal_rx_buffer);
                memset(internal_rx_buffer, 0, sizeof(internal_rx_buffer)); // 解析后清空
                internal_rx_index = 0;
            }
        }
    } else {
        // 缓冲区溢出，清空并重置
        printf("[ESP8266] RX buffer overflow, resetting...\r\n");
        memset(internal_rx_buffer, 0, sizeof(internal_rx_buffer));
        internal_rx_index = 0;
    }
}


//===================== 主控制接口 =====================//

/**
  * @brief  自动检查ESP8266状态，连接WiFi，连接MQTT，并订阅主题。
  * @retval ESP8266_OK: 整个连接流程成功。
  * @retval ESP8266_ERROR: 任何一步失败。
  */
ESP8266_Status_t ESP8266_Auto_CheckAndConnect(void)
{
    printf("[ESP8266] === Auto MQTT Connection Start ===\r\n");

    // 1. 发送AT命令，检查模块是否正常工作。
    //    如果第一次没有响应，尝试复位模块。
    ESP8266_Send_Command("AT", 100);
    if (ESP8266_Wait_Response("OK", 2000) != ESP8266_OK) {
        printf("[ESP8266] No response from AT, attempting reset...\r\n");
        ESP8266_Send_Command("AT+RST", 100);
        if (ESP8266_Wait_Response("ready", 5000) != ESP8266_OK) { // 成功复位后通常会输出 "ready"
            printf("[ESP8266] Module reset failed or no 'ready' after reset.\r\r\n");
            return ESP8266_ERROR;
        }
    }
    printf("[ESP8266] Module ready.\r\n");
    HAL_Delay(100); // 稍作延时，确保模块稳定

    // 2. 配置WiFi模式为 station 模式
    printf("[ESP8266] Setting WiFi mode to Station...\r\r\n");
    ESP8266_Send_Command("AT+CWMODE=1", 100);
    if (ESP8266_Wait_Response("OK", 3000) != ESP8266_OK) return ESP8266_ERROR;

    // 3. 连接到WiFi网络
    printf("[ESP8266] Connecting to WiFi: %s...\r\n", esp8266_config.wifi_ssid);
    char wifi_cmd[128];
    snprintf(wifi_cmd, sizeof(wifi_cmd), "AT+CWJAP=\"%s\",\"%s\"",
             esp8266_config.wifi_ssid, esp8266_config.wifi_password);
    ESP8266_Send_Command(wifi_cmd, 100);
    if (ESP8266_Wait_Response("WIFI GOT IP", 15000) != ESP8266_OK) { // WIFI GOT IP是连接成功的标志
        printf("[ESP8266] WiFi connection failed. Check SSID/password or signal.\r\n");
        return ESP8266_ERROR;
    }
    esp8266_current_state = ESP_STATE_WIFI_CONNECTED;
    printf("[ESP8266] WiFi Connected! Got IP.\r\r\n");
    HAL_Delay(500); // 确保网络稳定

    // 4. MQTT 连接流程
    if (ESP8266_Connect_MQTT() != ESP8266_OK) {
        printf("[ESP8266] MQTT connection failed.\r\n");
        return ESP8266_ERROR;
    }

    HAL_Delay(500); // 确保MQTT连接稳定
    if (ESP8266_Subscribe_Topic() != ESP8266_OK) {
        printf("[ESP8266] MQTT subscription failed.\r\n");
        return ESP8266_ERROR;
    }

    printf("[ESP8266] Ready to receive MQTT messages!\r\n");
    return ESP8266_OK;
}
