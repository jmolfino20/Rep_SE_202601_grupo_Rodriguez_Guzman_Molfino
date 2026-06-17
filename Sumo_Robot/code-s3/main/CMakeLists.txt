idf_component_register(
    SRCS "code-s3.c" "motor.c" "adc_audio.c" "fft.c"
         "state_machine.c" "uart_rx.c" "audio.c" "test_modes.c"
    INCLUDE_DIRS "."
    REQUIRES driver esp_adc esp-dsp freertos
             esp_driver_gpio esp_driver_ledc esp_driver_uart
             esp_timer nvs_flash
)
