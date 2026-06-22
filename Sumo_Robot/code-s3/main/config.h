#ifndef CONFIG_H
#define CONFIG_H

#include "driver/ledc.h"
#include "esp_adc/adc_oneshot.h"

/* ── Motors (L298N) ─────────────────────────────────────────────── */
#define MOTOR_IN1    17
#define MOTOR_IN2     8
#define MOTOR_IN3     9
#define MOTOR_IN4    10
#define MOTOR_ENA     4
#define MOTOR_ENB    11
#define MOTOR_PWM_FREQ   500
#define MOTOR_PWM_RES    LEDC_TIMER_8_BIT

/* ── UART from CamBorder (ESP32-CAM 1) ──────────────────────────── */
/* GPIO43/44 = UART0 consola ESP-IDF -- 39/38 evitan el conflicto    */
#define UART_BORDER_PORT   UART_NUM_1
#define UART_BORDER_RX     40
#define UART_BORDER_TX     41

/* ── UART from CamID (ESP32-CAM 2) ─────────────────────────────── */
/* GPIO15/16 ocupados por Flash Octal interno del N16R8 — no usables */
#define UART_ID_PORT       UART_NUM_2
#define UART_ID_RX         38
#define UART_ID_TX         39

/* ── Microphone (ADC, GPIO 1 = ADC1_CH0) ────────────────────────── */
#define AUDIO_ADC_CHANNEL  ADC_CHANNEL_0
#define SAMPLE_RATE        4000.0f
#define FFT_SIZE           1024
#define ADC_BITWIDTH_CFG   ADC_BITWIDTH_12

/* ── Button / switch (Attack trigger, active-low) ───────────────── */
#define BUTTON_PIN   2

/* ── State-machine timing (ms) ──────────────────────────────────── */
#define IDLE_FWD_MS              4000
#define IDLE_TURN_MS              200
#define ATTACK_FWD_MS             400
#define ATTACK_BACK_MS            500
#define FAST_SEARCH_TURN_MS       350
#define FAST_SEARCH_MAX_CYCLES      2

/* ── Motor duty values (0–255) ──────────────────────────────────── */
#define DUTY_IDLE_FWD       100
#define DUTY_DETECTED_FWD   120
#define DUTY_ATTACK_FWD     200
#define DUTY_ATTACK_BACK    100
#define DUTY_TURN           100
#define DUTY_FAST_TURN      100
#define DUTY_BORDER_TURN    100

/* ── Audio FFT ───────────────────────────────────────────────────── *
 * AUDIO_MIN_MAGNITUDE: umbral de magnitud del pico FFT para          *
 * considerar que hay una nota real. Si el pico está bajo este valor  *
 * se ignora (ruido de fondo). Subir si hay falsas detecciones,       *
 * bajar si no detecta notas aunque haya señal.                       *
 * ──────────────────────────────────────────────────────────────────*/
#define AUDIO_MIN_MAGNITUDE   0.00000003f

/* ── Audio note → command frequency bands (Hz) ──────────────────── */
#define AUDIO_FWD_LO     490.0f
#define AUDIO_FWD_HI     510.0f
#define AUDIO_BACK_LO    690.0f
#define AUDIO_BACK_HI    710.0f
#define AUDIO_LEFT_LO    850.0f
#define AUDIO_LEFT_HI    870.0f
#define AUDIO_RIGHT_LO  1180.0f
#define AUDIO_RIGHT_HI  1210.0f

/* ── Modo de operación ──────────────────────────────────────────── *
 *  Cambiar ROBOT_MODE para seleccionar qué probar:                  *
 *    MODE_COMPETITION  Comportamiento completo (competencia)         *
 *    MODE_AUDIO        Solo micrófono/FFT                           *
 *    MODE_BORDER       Solo cámara de borde                         *
 *    MODE_ID           Solo cámara de identificación                *
 *    MODE_BUTTON       Solo botón de ataque                         *
 * ──────────────────────────────────────────────────────────────── */
#define MODE_COMPETITION  0
#define MODE_AUDIO        1
#define MODE_BORDER       2
#define MODE_ID           3
#define MODE_BUTTON       4

#define ROBOT_MODE MODE_COMPETITION

#endif /* CONFIG_H */
