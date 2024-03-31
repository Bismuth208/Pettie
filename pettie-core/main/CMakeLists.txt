set(MAIN_SRCS
    "main.c"
    )

set(WIRELESS_MODULE_SRCS
    "wireless/wireless_main.c"
    )

set(MQTT_MODULE_SRCS
    "mqtt/mqtt_func.c"
    )

set(SERVO_MODULE_SRCS
    "servo/servo.c"
    "servo/servo_pwm.c"
    )

set(MEMORY_MODEL_SRCS
    "memory_model/memory_model.c"
    "memory_model/memory_model_topics.c"
    )

set(PETTIE_SRCS
    "neurons_list.c"
    "pettie_core.c"
    )

set(MISC_SRCS
    "common/hash_functions.c"
    "common/common_func.c"
    )

idf_component_register(SRCS
    ${MAIN_SRCS}
    ${WIRELESS_MODULE_SRCS}
    ${MQTT_MODULE_SRCS}
    ${SERVO_MODULE_SRCS}
    ${MEMORY_MODEL_SRCS}
    ${PETTIE_SRCS}
    ${MISC_SRCS}
    INCLUDE_DIRS ".")

target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")