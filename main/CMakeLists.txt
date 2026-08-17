
file(GLOB_RECURSE srcs *.c)

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS "." "../components/at/include")

target_link_libraries(${COMPONENT_LIB} INTERFACE "-u esp_at_custom_cmd_register")
