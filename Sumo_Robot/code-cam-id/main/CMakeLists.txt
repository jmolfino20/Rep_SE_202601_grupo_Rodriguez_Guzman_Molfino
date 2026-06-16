idf_component_register(
    SRCS "code-cam-id.c" "ml_id.c"
    INCLUDE_DIRS "."
    REQUIRES esp32-camera driver esp_driver_uart freertos
)
