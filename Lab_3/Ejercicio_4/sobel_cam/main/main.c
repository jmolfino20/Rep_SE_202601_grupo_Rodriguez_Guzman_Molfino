#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_camera.h"
#include "sobel.h"
#include "driver/uart.h"

// Pines AI-Thinker
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
#define UART_PORT UART_NUM_1
#define UART_TX   1
#define UART_RX   3
#define UART_BAUD 115200

static void init_uart() {
    uart_config_t cfg = {
        .baud_rate  = UART_BAUD,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_DISABLE,
        .stop_bits  = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
    };
    uart_driver_install(UART_PORT, 256, 0, 0, NULL, 0);
    uart_param_config(UART_PORT, &cfg);
    uart_set_pin(UART_PORT, UART_TX, UART_RX, -1, -1);
}

static void init_camera() {
    camera_config_t cfg = {
        .pin_pwdn     = PWDN_GPIO_NUM,
        .pin_reset    = RESET_GPIO_NUM,
        .pin_xclk     = XCLK_GPIO_NUM,
        .pin_sscb_sda = SIOD_GPIO_NUM,
        .pin_sscb_scl = SIOC_GPIO_NUM,
        .pin_d7 = Y9_GPIO_NUM, .pin_d6 = Y8_GPIO_NUM,
        .pin_d5 = Y7_GPIO_NUM, .pin_d4 = Y6_GPIO_NUM,
        .pin_d3 = Y5_GPIO_NUM, .pin_d2 = Y4_GPIO_NUM,
        .pin_d1 = Y3_GPIO_NUM, .pin_d0 = Y2_GPIO_NUM,
        .pin_vsync    = VSYNC_GPIO_NUM,
        .pin_href     = HREF_GPIO_NUM,
        .pin_pclk     = PCLK_GPIO_NUM,
        .xclk_freq_hz = 20000000,
        .pixel_format = PIXFORMAT_GRAYSCALE,
        .frame_size   = FRAMESIZE_96X96,  // 96x96
        .fb_count     = 1,
        .fb_location  = CAMERA_FB_IN_DRAM,
    };

    esp_err_t err = esp_camera_init(&cfg);
    if (err != ESP_OK) {
        printf("Camera init failed: 0x%x\n", err);
        return;
    }
    printf("Camera OK\n");
}

void app_main(void) {
    init_camera();
    init_uart();

    while (1) {
        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb) {
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }

        BordeZonas z = sobel_detectar(fb->buf, fb->width, fb->height);
        NavCmd cmd = sobel_decidir(z);
        esp_camera_fb_return(fb);

        uint8_t byte = (uint8_t)cmd;
        uart_write_bytes(UART_PORT, &byte, 1);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}