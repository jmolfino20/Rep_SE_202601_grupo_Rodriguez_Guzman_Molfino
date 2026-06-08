#include "uart_cmd.h"
#include "motor.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

#define UART_PORT   UART_NUM_1
#define UART_RX_PIN 44
#define UART_TX_PIN 43
#define UART_BAUD   115200


static void uart_task(void *arg)
{
    uint8_t byte;
    

    while (1) {
        int n = uart_read_bytes(UART_PORT, &byte, 1,
                                pdMS_TO_TICKS(100));
        if (n > 0) {
            
            printf("CMD: %d\n", byte);

            switch (byte) {
                case 0: motor_stop();              break;
                case 3: motor_forward(100, 100);   break;
                case 7: motor_backward(100, 100);  break;
                case 1: motor_left(90, 90);     break;  // gira izq
                case 5: motor_right(90, 90);     break;  // gira der
                default: break;
            }
        }

    }
}

void uart_cmd_init(void)
{
    uart_config_t cfg = {
        .baud_rate  = UART_BAUD,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_DISABLE,
        .stop_bits  = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
    };
    uart_driver_install(UART_PORT, 256, 0, 0, NULL, 0);
    uart_param_config(UART_PORT, &cfg);
    uart_set_pin(UART_PORT, UART_TX_PIN, UART_RX_PIN, -1, -1);

    xTaskCreate(uart_task, "uart_task", 2048, NULL, 5, NULL);
}