idf_component_register(
    SRCS "code-cam-border.c" "sobel.c"
    INCLUDE_DIRS "."
    REQUIRES esp32-camera driver esp_driver_uart freertos
)
