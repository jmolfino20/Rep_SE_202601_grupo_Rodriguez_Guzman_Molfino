#include "uart_rx.h"
#include "globals.h"
#include "config.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* Fix #2: orden correcto param_config → set_pin → driver_install */
static void uart_install(uart_port_t port, int rx, int tx) {
    uart_config_t cfg = {
        .baud_rate  = 115200,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_DISABLE,
        .stop_bits  = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
    };
    uart_param_config(port, &cfg);
    uart_set_pin(port, tx, rx, -1, -1);
    uart_driver_install(port, 256, 0, 0, NULL, 0);
}

/* Fix #3: inits separados para poder llamar solo el necesario por modo */
void uart_border_init(void) {
    uart_install(UART_BORDER_PORT, UART_BORDER_RX, UART_BORDER_TX);
}

void uart_id_init(void) {
    uart_install(UART_ID_PORT, UART_ID_RX, UART_ID_TX);
}

void uart_rx_init(void) {
    uart_border_init();
    uart_id_init();
}

/*
 * Fix #1 y #7: logs completos para distinguir:
 *   - Primera conexión establecida
 *   - Cambio de estado (byte recibido distinto al anterior)
 *   - Timeout (500 ms sin datos) → posible cámara desconectada
 *   - Reconexión tras timeouts consecutivos
 *
 * Se loguea solo en cambios de estado para no saturar el monitor.
 * En timeout se avisa la primera vez y luego cada 10 (= cada 5 s).
 */
void uart_border_task(void *arg) {
    uint8_t byte;
    uint8_t  prev      = 0xFF;   /* 0xFF = estado inicial desconocido */
    uint32_t timeouts  = 0;
    uint8_t  first_rx  = 1;

    while (1) {
        int n = uart_read_bytes(UART_BORDER_PORT, &byte, 1, pdMS_TO_TICKS(500));

        if (n > 0) {
            if (first_rx) {
                ESP_LOGI("UART_BORDER", "comunicacion establecida (UART_NUM_%d, GPIO%d)",
                         (int)UART_BORDER_PORT, UART_BORDER_RX);
                first_rx = 0;
            }
            if (timeouts > 0) {
                ESP_LOGI("UART_BORDER", "comunicacion restablecida tras %u timeouts", timeouts);
                timeouts = 0;
            }
            g_border_detected = byte;
            if (byte != prev) {
                ESP_LOGI("UART_BORDER", "rx=0x%02X -> borde=%s",
                         byte, byte ? "DETECTADO" : "libre");
                prev = byte;
            }
        } else {
            g_border_detected = 0;
            timeouts++;
            if (timeouts == 1) {
                ESP_LOGW("UART_BORDER", "timeout (500 ms sin datos) -- CamBorder desconectada?");
            } else if (timeouts % 10 == 0) {
                ESP_LOGW("UART_BORDER", "%u timeouts consecutivos (%.0f s sin comunicacion)",
                         timeouts, timeouts * 0.5f);
            }
            if (prev != 0xFF) prev = 0xFF;
        }
    }
}

void uart_id_task(void *arg) {
    uint8_t byte;
    uint8_t  prev      = 0xFF;
    uint32_t timeouts  = 0;
    uint8_t  first_rx  = 1;

    while (1) {
        int n = uart_read_bytes(UART_ID_PORT, &byte, 1, pdMS_TO_TICKS(500));

        if (n > 0) {
            if (first_rx) {
                ESP_LOGI("UART_ID", "comunicacion establecida (UART_NUM_%d, GPIO%d)",
                         (int)UART_ID_PORT, UART_ID_RX);
                first_rx = 0;
            }
            if (timeouts > 0) {
                ESP_LOGI("UART_ID", "comunicacion restablecida tras %u timeouts", timeouts);
                timeouts = 0;
            }
            g_id_detected = byte;
            if (byte != prev) {
                ESP_LOGI("UART_ID", "rx=0x%02X -> ID=%s",
                         byte, byte ? "DETECTADO" : "libre");
                prev = byte;
            }
        } else {
            g_id_detected = 0;
            timeouts++;
            if (timeouts == 1) {
                ESP_LOGW("UART_ID", "timeout (500 ms sin datos) -- CamID desconectada?");
            } else if (timeouts % 10 == 0) {
                ESP_LOGW("UART_ID", "%u timeouts consecutivos (%.0f s sin comunicacion)",
                         timeouts, timeouts * 0.5f);
            }
            if (prev != 0xFF) prev = 0xFF;
        }
    }
}
