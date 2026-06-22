#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_camera.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "ml_id.h"
#include "config.h"

#define TAG "CAM_ID"

/* Sentinel UART: indica "sin lectura valida" (error de inferencia o tamano
 * de frame incorrecto). Las clases reales van de 0 a 3. */
#define CLASS_INVALID 0xFF

static QueueHandle_t g_queue;   /* mailbox de 1 byte entre los dos cores */

static void camera_init(void) {
    camera_config_t cfg = {
        .pin_pwdn     = CAM_PIN_PWDN,
        .pin_reset    = CAM_PIN_RESET,
        .pin_xclk     = CAM_PIN_XCLK,
        .pin_sscb_sda = CAM_PIN_SIOD,
        .pin_sscb_scl = CAM_PIN_SIOC,
        .pin_d7 = CAM_PIN_D7, .pin_d6 = CAM_PIN_D6,
        .pin_d5 = CAM_PIN_D5, .pin_d4 = CAM_PIN_D4,
        .pin_d3 = CAM_PIN_D3, .pin_d2 = CAM_PIN_D2,
        .pin_d1 = CAM_PIN_D1, .pin_d0 = CAM_PIN_D0,
        .pin_vsync    = CAM_PIN_VSYNC,
        .pin_href     = CAM_PIN_HREF,
        .pin_pclk     = CAM_PIN_PCLK,
        .xclk_freq_hz = 20000000,
        .pixel_format = PIXFORMAT_GRAYSCALE,
        .frame_size   = FRAMESIZE_QVGA,
        .fb_count     = 1,
        .fb_location  = CAMERA_FB_IN_DRAM,
    };
    ESP_ERROR_CHECK(esp_camera_init(&cfg));
    ESP_LOGI(TAG, "camara inicializada OK (QVGA %dx%d, escala de grises)", CAM_WIDTH, CAM_HEIGHT);
}

static void uart_init(void) {
    uart_config_t cfg = {
        .baud_rate  = UART_BAUD,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_DISABLE,
        .stop_bits  = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
    };
    uart_param_config(UART_PORT, &cfg);
    uart_set_pin(UART_PORT, UART_TX_PIN, UART_RX_PIN, -1, -1);
    uart_driver_install(UART_PORT, 256, 0, 0, NULL, 0);
    ESP_LOGI(TAG, "UART inicializado OK (UART_NUM_%d, TX=GPIO%d, baud=%d)",
             (int)UART_PORT, UART_TX_PIN, UART_BAUD);
}

/* Core 0: captura frame y ejecuta inferencia ML */
static void id_detect_task(void *arg) {
    uint32_t frames     = 0;
    uint32_t fb_errors  = 0;

    ESP_LOGI(TAG, "id_detect_task iniciada (periodo=%d ms)", DETECT_PERIOD_MS);

    while (1) {
        camera_fb_t *fb = esp_camera_fb_get();
        if (fb) {
            int class_id = ml_id_detect(fb->buf, fb->width, fb->height);
            esp_camera_fb_return(fb);

            uint8_t result = (class_id >= 0) ? (uint8_t)class_id : CLASS_INVALID;
            xQueueOverwrite(g_queue, &result);
            frames++;

            /* Heartbeat cada 100 frames (~5 s a 50 ms/frame) */
            if (frames % 100 == 0) {
                ESP_LOGI(TAG, "MONITOR | frames=%u  fb_errors=%u  ultima_clase=%d",
                         frames, fb_errors, class_id);
            }
        } else {
            fb_errors++;
            ESP_LOGW(TAG, "esp_camera_fb_get fallo (total=%u)", fb_errors);
        }
        vTaskDelay(pdMS_TO_TICKS(DETECT_PERIOD_MS));
    }
}

/* Core 1: envía el último resultado por UART al S3 */
static void uart_send_task(void *arg) {
    uint8_t val  = CLASS_INVALID;
    uint8_t prev = CLASS_INVALID;

    ESP_LOGI(TAG, "uart_send_task iniciada");

    while (1) {
        if (xQueuePeek(g_queue, &val, pdMS_TO_TICKS(DETECT_PERIOD_MS * 2))) {
            uart_write_bytes(UART_PORT, &val, 1);

            /* Loguear solo en cambio de valor */
            if (val != prev) {
                if (val == CLASS_INVALID)
                    ESP_LOGI(TAG, "UART tx=0x%02X (sin lectura valida)", val);
                else
                    ESP_LOGI(TAG, "UART tx=0x%02X (clase=%u)", val, val);
                prev = val;
            }
        }
        /* Sin este delay la task loopea a velocidad maxima, llenando el buffer
         * UART del S3 a 11520 bytes/s con el mismo byte repetido. */
        vTaskDelay(pdMS_TO_TICKS(DETECT_PERIOD_MS));
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "=== CamID iniciando ===");
    camera_init();
    ml_id_init();
    ESP_LOGI(TAG, "Init completo. Remapeando UART — consola se pierde despues de esto.");
    vTaskDelay(pdMS_TO_TICKS(100));  /* dejar que el log flush antes de perder la consola */
    uart_init();

    g_queue = xQueueCreate(1, sizeof(uint8_t));

    xTaskCreatePinnedToCore(id_detect_task, "id_detect", 8192, NULL, 5, NULL, 0);
    xTaskCreatePinnedToCore(uart_send_task, "uart_send", 2048, NULL, 5, NULL, 1);
}