file(GLOB_RECURSE srcs *.c)
set(includes "include")

# 将 driver 改为 esp8266
set(require_components at freertos nvs_flash esp8266 esp_event esp_wifi)

idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS ${includes}
    REQUIRES ${require_components})

target_link_libraries(${COMPONENT_LIB} INTERFACE "-u esp_at_custom_cmd_register_wifiled")