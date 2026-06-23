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
#define SAMPLE_RATE        8000.0f
#define FFT_SIZE           1024
#define ADC_BITWIDTH_CFG   ADC_BITWIDTH_12

/* ── Button / switch (Attack trigger, active-low) ───────────────── */
#define BUTTON_PIN   2

/* ── State-machine timing (ms) ──────────────────────────────────── */
#define IDLE_FWD_MS              1500
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

/* ── Audio DTMF — comandos por par de frecuencias ───────────────────
 *
 *  Comando     F1 (fila)   F2 (columna)
 *  ATAQUE      697 Hz      1336 Hz
 *  DERECHA     770 Hz      1633 Hz
 *  IZQUIERDA   852 Hz      1209 Hz
 *  ATRÁS       941 Hz      1477 Hz
 * ─────────────────────────────────────────────────────────────────*/
#define AUDIO_TOLERANCE     100.0f   /* ± Hz aceptado por banda */

#define AUDIO_A_F1   697.0f         /* ATAQUE     */
#define AUDIO_A_F2  1836.0f

#define AUDIO_6_F1   770.0f         /* DERECHA    */
#define AUDIO_6_F2  1633.0f

#define AUDIO_4_F1   852.0f         /* IZQUIERDA  */
#define AUDIO_4_F2  1209.0f

#define AUDIO_9_F1   941.0f         /* ATRÁS      */
#define AUDIO_9_F2  1477.0f
 
#define AUDIO_MIN_MAGNITUDE     0.35f       /* filtra ruido (~0.1-0.28) vs señal real (~0.5+) */
#define AUDIO_CONFIRM_WINDOW    3           /* ventana de frames para evaluar */
#define AUDIO_CONFIRM_NEEDED    2          /* cuantos de la ventana deben coincidir */

// /* ── Audio note → command frequency bands (Hz) ──────────────────── */
// #define AUDIO_FWD_LO     490.0f
// #define AUDIO_FWD_HI     510.0f
// #define AUDIO_BACK_LO    690.0f
// #define AUDIO_BACK_HI    710.0f
// #define AUDIO_LEFT_LO    850.0f
// #define AUDIO_LEFT_HI    870.0f
// #define AUDIO_RIGHT_LO  1180.0f
// #define AUDIO_RIGHT_HI  1210.0f

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
