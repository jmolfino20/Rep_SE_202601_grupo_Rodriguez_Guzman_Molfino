#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "nvs_flash.h"
#include "globals.h"
#include "motor.h"
#include "state_machine.h"
#include "uart_rx.h"
#include "audio.h"
#include "config.h"

/* Definiciones de los flags globales (declarados extern en globals.h) */
volatile uint8_t g_id_detected     = 0;
volatile uint8_t g_border_detected = 0;
volatile uint8_t g_audio_override  = 0;

void app_main(void) {
    nvs_flash_init();

    /* Botón de ataque (activo en bajo) */
    gpio_config_t btn = {
        .pin_bit_mask = (1ULL << BUTTON_PIN),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    gpio_config(&btn);

    motor_init();
    uart_rx_init();

    /* Core 0 – control de motores y máquina de estados */
    xTaskCreatePinnedToCore(state_machine_task, "state_machine", 4096, NULL, 4, NULL, 0);
    /* Core 0 – recepción UART de cámaras (menor prioridad) */
    xTaskCreatePinnedToCore(uart_border_task,   "uart_border",   2048, NULL, 3, NULL, 0);
    xTaskCreatePinnedToCore(uart_id_task,        "uart_id",       2048, NULL, 3, NULL, 0);
    /* Core 1 – audio y FFT */
    xTaskCreatePinnedToCore(audio_task,          "audio",         8192, NULL, 5, NULL, 1);
}
