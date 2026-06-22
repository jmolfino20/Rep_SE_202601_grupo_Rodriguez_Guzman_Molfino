#include "test_modes.h"
#include "globals.h"
#include "motor.h"
#include "config.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* ── MODE_BORDER ─────────────────────────────────────────────────────
 * Avanza hacia adelante. Cuando la CamBorder detecta el borde blanco
 * gira a la derecha hasta despejarlo.
 *
 * Monitor: imprime estado cada 500 ms (50 ticks × 10 ms).
 */
void test_border_task(void *arg) {
    ESP_LOGI("TEST_BORDER", "Iniciando: avanza hasta borde, gira derecha");
    ESP_LOGI("TEST_BORDER", "Esperando datos de CamBorder por UART_NUM_%d (RX=GPIO%d)",
             (int)UART_BORDER_PORT, UART_BORDER_RX);

    uint8_t in_border   = 0;
    uint32_t tick_count = 0;

    while (1) {
        /* Monitor periódico cada 500 ms */
        if (tick_count % 50 == 0) {
            ESP_LOGI("TEST_BORDER", "MONITOR | g_border_detected=%d | motor=%s",
                     g_border_detected,
                     g_border_detected ? "GIRANDO DER" : "ADELANTE");
        }
        tick_count++;

        if (g_border_detected) {
            if (!in_border) {
                in_border = 1;
                ESP_LOGW("TEST_BORDER", "Borde detectado → girando derecha (duty=%d)", DUTY_BORDER_TURN);
            }
            motor_right(DUTY_BORDER_TURN, DUTY_BORDER_TURN);
        } else {
            if (in_border) {
                in_border = 0;
                ESP_LOGI("TEST_BORDER", "Borde despejado → avanzando (duty=%d)", DUTY_IDLE_FWD);
            }
            motor_forward(DUTY_IDLE_FWD, DUTY_IDLE_FWD);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/* ── MODE_ID ─────────────────────────────────────────────────────────
 * Binario: 0=Sin ID, 1=Con ID.
 * Avanza cuando detecta el identificador, se detiene si lo pierde.
 */
void test_id_task(void *arg) {
    ESP_LOGI("TEST_ID", "Iniciando: avanza si hay ID, para si no hay");
    ESP_LOGI("TEST_ID", "Esperando datos de CamID por UART_NUM_%d (RX=GPIO%d)",
             (int)UART_ID_PORT, UART_ID_RX);

    uint8_t  prev       = 0xFF;
    uint32_t tick_count = 0;

    while (1) {
        uint8_t det = g_id_detected;

        if (tick_count % 50 == 0) {
            ESP_LOGI("TEST_ID", "MONITOR | g_id_detected=%d | motor=%s",
                     det, det ? "ADELANTE" : "DETENIDO");
        }
        tick_count++;

        if (det != prev) {
            ESP_LOGI("TEST_ID", "ID %s", det ? "DETECTADO -> adelante" : "PERDIDO -> detenido");
            prev = det;
        }

        if (det) {
            motor_forward(DUTY_DETECTED_FWD, DUTY_DETECTED_FWD);
        } else {
            motor_stop();
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/* ── MODE_BUTTON ─────────────────────────────────────────────────────
 * Avanza mientras el botón está presionado, para al soltarlo.
 *
 * Monitor: imprime estado cada 200 ms.
 */
void test_button_task(void *arg) {
    ESP_LOGI("TEST_BTN", "Iniciando: mantén botón (GPIO %d, activo en bajo) para avanzar", BUTTON_PIN);

    uint8_t  prev       = 0;
    uint32_t tick_count = 0;

    while (1) {
        uint8_t pressed = (gpio_get_level(BUTTON_PIN) == 0);

        /* Monitor periódico cada 200 ms */
        if (tick_count % 20 == 0) {
            ESP_LOGI("TEST_BTN", "MONITOR | boton=%s (GPIO%d=%d) | motor=%s",
                     pressed ? "PRESIONADO" : "suelto",
                     BUTTON_PIN, gpio_get_level(BUTTON_PIN),
                     pressed ? "ADELANTE" : "DETENIDO");
        }
        tick_count++;

        if (pressed) {
            if (!prev) {
                prev = 1;
                ESP_LOGW("TEST_BTN", "Botón presionado → adelante (duty=%d)", DUTY_ATTACK_FWD);
            }
            motor_forward(DUTY_ATTACK_FWD, DUTY_ATTACK_FWD);
        } else {
            if (prev) {
                prev = 0;
                ESP_LOGI("TEST_BTN", "Botón suelto → detenido");
            }
            motor_stop();
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
