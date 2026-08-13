/*
 * SPDX-FileCopyrightText: 2024-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "esp_at.h"
#include "driver/gpio.h"
#include "esp_event.h"
#include "esp_wifi.h"

static int wifi_led_gpio = -1;

// Wi-Fi 事件回调
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (wifi_led_gpio < 0) return;

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
        gpio_set_level(wifi_led_gpio, 1);
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        gpio_set_level(wifi_led_gpio, 0);
    }
}

static void register_wifi_event_listener(void)
{
    // 修改：使用 esp_event_handler_register 替代 esp_event_handler_instance_register
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
}

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

    if (pin < 0 || pin > 16 || pin == 1 || pin == 3 ||
        pin == 6 || pin == 7 || pin == 8 || pin == 9 || pin == 10 || pin == 11) {
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

// AT+WIFILED 执行命令
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

// 命令注册表
static const esp_at_cmd_struct at_custom_cmd[] = {
    {"+WIFILED", at_test_cmd_wifiled, at_query_cmd_wifiled,
     at_setup_cmd_wifiled, at_exe_cmd_wifiled},
};

bool esp_at_custom_cmd_register_wifiled(void)
{
    return esp_at_custom_cmd_array_regist(at_custom_cmd,
        sizeof(at_custom_cmd) / sizeof(at_custom_cmd[0]));
}

// 修改：去掉第二个参数，或者尝试 0
ESP_AT_CMD_SET_INIT_FN(esp_at_custom_cmd_register_wifiled);