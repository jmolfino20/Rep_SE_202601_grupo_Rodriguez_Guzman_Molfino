#ifndef CONFIG_H
#define CONFIG_H

/* AI-Thinker ESP32-CAM camera pins */
#define CAM_PIN_PWDN    32
#define CAM_PIN_RESET   -1
#define CAM_PIN_XCLK     0
#define CAM_PIN_SIOD    26
#define CAM_PIN_SIOC    27
#define CAM_PIN_D7      35
#define CAM_PIN_D6      34
#define CAM_PIN_D5      39
#define CAM_PIN_D4      36
#define CAM_PIN_D3      21
#define CAM_PIN_D2      19
#define CAM_PIN_D1      18
#define CAM_PIN_D0       5
#define CAM_PIN_VSYNC   25
#define CAM_PIN_HREF    23
#define CAM_PIN_PCLK    22

/* UART to ESP32-S3 */
#define UART_PORT       UART_NUM_1
#define UART_TX_PIN      1
#define UART_RX_PIN      3
#define UART_BAUD       115200

/* Captura QVGA (320x240), luego crop+resize a 128x128 en preprocesamiento */
#define CAM_WIDTH       320
#define CAM_HEIGHT      240
#define MODEL_INPUT     128

/* Frame capture period (ms) */
#define DETECT_PERIOD_MS  50

#endif /* CONFIG_H */
