idf_component_register(
                    SRCS "Ejercicio_5.c" "adc_audio.c" "fft.c" "motor.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp-dsp driver esp_adc esp_driver_gpio esp_driver_ledc esp_timer
)
