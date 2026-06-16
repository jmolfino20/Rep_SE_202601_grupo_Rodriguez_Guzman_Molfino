#include "uart_rx.h"
#include "globals.h"
#include "config.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static void uart_install(uart_port_t port, int rx, int tx) {
    uart_config_t cfg = {
        .baud_rate  = 115200,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_DISABLE,
        .stop_bits  = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
    };
    uart_driver_install(port, 256, 0, 0, NULL, 0);
    uart_param_config(port, &cfg);
    uart_set_pin(port, tx, rx, -1, -1);
}

void uart_rx_init(void) {
    uart_install(UART_BORDER_PORT, UART_BORDER_RX, UART_BORDER_TX);
    uart_install(UART_ID_PORT,     UART_ID_RX,     UART_ID_TX);
}

/*
 * Timeout 500 ms: si la CamBorder no envía datos, asumimos sin borde.
 * La CamBorder envía 0x00 o 0x01 cada ~30 ms.
 */
void uart_border_task(void *arg) {
    uint8_t byte;
    while (1) {
        int n = uart_read_bytes(UART_BORDER_PORT, &byte, 1, pdMS_TO_TICKS(500));
        g_border_detected = (n > 0) ? byte : 0;
    }
}

void uart_id_task(void *arg) {
    uint8_t byte;
    while (1) {
        int n = uart_read_bytes(UART_ID_PORT, &byte, 1, pdMS_TO_TICKS(500));
        g_id_detected = (n > 0) ? byte : 0;
    }
}
