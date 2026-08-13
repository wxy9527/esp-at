file(GLOB_RECURSE srcs *.c)
set(includes "include")

# ESP8266 只需要依赖 at、freertos、nvs_flash 和 esp8266
# esp_wifi、esp_event、driver 都包含在 esp8266 组件中
set(require_components at freertos nvs_flash esp8266)

idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS ${includes}
    REQUIRES ${require_components})

target_link_libraries(${COMPONENT_LIB} INTERFACE "-u esp_at_custom_cmd_register_wifiled")