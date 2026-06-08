idf_component_register(
    SRCS "esp-ble.c" "motor.c" "uart_cmd.c"
    INCLUDE_DIRS "."
    REQUIRES driver freertos nvs_flash esp_driver_gpio esp_driver_ledc esp_driver_uart
)