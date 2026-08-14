#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "esp_at.h"
#include "driver/gpio.h"
#include "esp_event.h"
#include "esp_wifi.h"

// 存储要控制的 GPIO 号，-1 表示未设置
static int wifi_led_gpio = -1;

// ==================== Wi-Fi 事件回调函数 ====================
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (wifi_led_gpio < 0) {
        return;
    }

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
        gpio_set_level(wifi_led_gpio, 1);
        printf("WiFi Connected! GPIO%d HIGH\n", wifi_led_gpio);
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        gpio_set_level(wifi_led_gpio, 0);
        printf("WiFi Disconnected! GPIO%d LOW\n", wifi_led_gpio);
    }
}

static void register_wifi_event_listener(void)
{
    esp_event_handler_register(WIFI_EVENT,
                               ESP_EVENT_ANY_ID,
                               wifi_event_handler,
                               NULL);
}

// ==================== AT 命令处理函数 ====================

// AT+WIFILED=? 测试命令
static uint8_t at_test_cmd_wifiled(uint8_t *cmd_name)
{
    esp_at_port_write_data((uint8_t *)"OK\r\n", 4);
    return ESP_AT_RESULT_CODE_OK;
}

// AT+WIFILED? 查询命令
static uint8_t at_query_cmd_wifiled(uint8_t *cmd_name)
{
    uint8_t buffer[32];
    snprintf((char *)buffer, sizeof(buffer), "+WIFILED:%d\r\n", wifi_led_gpio);
    esp_at_port_write_data(buffer, strlen((char *)buffer));
    return ESP_AT_RESULT_CODE_OK;
}

// AT+WIFILED=<pin> 设置命令
static uint8_t at_setup_cmd_wifiled(uint8_t para_num)
{
    int32_t pin = 0;
    if (esp_at_get_para_as_digit(0, &pin) != ESP_AT_PARA_PARSE_RESULT_OK) {
        return ESP_AT_RESULT_CODE_ERROR;
    }

    // 校验 GPIO 号合法性（ESP32-S2 可用 GPIO: 0-21, 26-46）
    if (pin < 0 || pin > 46) {
        return ESP_AT_RESULT_CODE_ERROR;
    }
    // 排除日志端口 (GPIO43/44) 和 USB 引脚 (GPIO19/20)
    if (pin == 19 || pin == 20 || pin == 43 || pin == 44) {
        return ESP_AT_RESULT_CODE_ERROR;
    }

    wifi_led_gpio = (int)pin;

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << wifi_led_gpio),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
    gpio_set_level(wifi_led_gpio, 0);

    static bool registered = false;
    if (!registered) {
        register_wifi_event_listener();
        registered = true;
    }

    return ESP_AT_RESULT_CODE_OK;
}

// AT+WIFILED 执行命令（手动拉高测试用）
static uint8_t at_exe_cmd_wifiled(uint8_t *cmd_name)
{
    if (wifi_led_gpio >= 0) {
        gpio_set_level(wifi_led_gpio, 1);
    }

    uint8_t buffer[32];
    snprintf((char *)buffer, sizeof(buffer), "GPIO:%d\r\n", wifi_led_gpio);
    esp_at_port_write_data(buffer, strlen((char *)buffer));

    return ESP_AT_RESULT_CODE_OK;
}

// ==================== 命令注册表 ====================
static const esp_at_cmd_struct at_custom_cmd[] = {
    {"+WIFILED", at_test_cmd_wifiled, at_query_cmd_wifiled,
     at_setup_cmd_wifiled, at_exe_cmd_wifiled},
};

bool esp_at_custom_cmd_register(void)
{
    return esp_at_custom_cmd_array_regist(at_custom_cmd,
        sizeof(at_custom_cmd) / sizeof(at_custom_cmd[0]));
}

ESP_AT_CMD_SET_INIT_FN(esp_at_custom_cmd_register);