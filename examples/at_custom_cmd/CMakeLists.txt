file(GLOB_RECURSE srcs *.c)
set(includes "include")

set(require_components at freertos nvs_flash driver esp_event esp_wifi)

idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS ${includes}
    REQUIRES ${require_components})

target_link_libraries(${COMPONENT_LIB} INTERFACE "-u esp_at_custom_cmd_register")