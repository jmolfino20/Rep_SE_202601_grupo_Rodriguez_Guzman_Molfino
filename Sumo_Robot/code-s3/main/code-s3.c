#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "globals.h"
#include "motor.h"
#include "state_machine.h"
#include "uart_rx.h"
#include "audio.h"
#include "test_modes.h"
#include "config.h"

#define TAG "MAIN"

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

#if ROBOT_MODE == MODE_AUDIO
    /* ── Prueba micrófono ────────────────────────────────────────────
     * Solo audio_task corre. Las notas 490/700/900/1100 Hz mueven el
     * robot. Ver logs "AUDIO" en monitor serie. */
    ESP_LOGI(TAG, "=== Modo: AUDIO (prueba micrófono) ===");
    xTaskCreatePinnedToCore(audio_task, "audio", 8192, NULL, 5, NULL, 1);

#elif ROBOT_MODE == MODE_BORDER
    /* ── Prueba CamBorder ────────────────────────────────────────────
     * Avanza recto; al detectar borde gira derecha hasta despejarlo.
     * Verificar logs "TEST_BORDER" y que UART_NUM_1 recibe datos. */
    ESP_LOGI(TAG, "=== Modo: BORDER (prueba CamBorder) ===");
    uart_rx_init();
    xTaskCreatePinnedToCore(uart_border_task, "uart_border", 2048, NULL, 3, NULL, 0);
    xTaskCreatePinnedToCore(test_border_task, "test_border", 4096, NULL, 4, NULL, 0);

#elif ROBOT_MODE == MODE_ID
    /* ── Prueba CamID ────────────────────────────────────────────────
     * Avanza cuando la cámara ve el identificador, para si lo pierde.
     * Verificar logs "TEST_ID" y que UART_NUM_2 recibe datos. */
    ESP_LOGI(TAG, "=== Modo: ID (prueba CamID) ===");
    uart_rx_init();
    xTaskCreatePinnedToCore(uart_id_task, "uart_id", 2048, NULL, 3, NULL, 0);
    xTaskCreatePinnedToCore(test_id_task, "test_id", 4096, NULL, 4, NULL, 0);

#elif ROBOT_MODE == MODE_BUTTON
    /* ── Prueba botón ────────────────────────────────────────────────
     * Avanza mientras el botón esté presionado, para al soltar.
     * Verificar logs "TEST_BTN" y movimiento del robot. */
    ESP_LOGI(TAG, "=== Modo: BUTTON (prueba botón) ===");
    xTaskCreatePinnedToCore(test_button_task, "test_btn", 4096, NULL, 4, NULL, 0);

#else /* MODE_COMPETITION */
    /* ── Competencia ─────────────────────────────────────────────────
     * Máquina de estados completa con ambas cámaras, audio y botón. */
    ESP_LOGI(TAG, "=== Modo: COMPETENCIA ===");
    uart_rx_init();
    xTaskCreatePinnedToCore(state_machine_task, "state_machine", 4096, NULL, 4, NULL, 0);
    xTaskCreatePinnedToCore(uart_border_task,   "uart_border",   2048, NULL, 3, NULL, 0);
    xTaskCreatePinnedToCore(uart_id_task,        "uart_id",       2048, NULL, 3, NULL, 0);
    xTaskCreatePinnedToCore(audio_task,          "audio",         8192, NULL, 5, NULL, 1);
#endif
}
