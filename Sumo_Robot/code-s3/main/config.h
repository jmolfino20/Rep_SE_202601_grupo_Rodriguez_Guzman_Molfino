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
#define UART_BORDER_PORT   UART_NUM_1
#define UART_BORDER_RX     44
#define UART_BORDER_TX     43

/* ── UART from CamID (ESP32-CAM 2) ─────────────────────────────── */
#define UART_ID_PORT       UART_NUM_2
#define UART_ID_RX         16
#define UART_ID_TX         15

/* ── Microphone (ADC, GPIO 1 = ADC1_CH0) ────────────────────────── */
#define AUDIO_ADC_CHANNEL  ADC_CHANNEL_0
#define SAMPLE_RATE        4000.0f
#define FFT_SIZE           1024
#define ADC_BITWIDTH_CFG   ADC_BITWIDTH_12

/* ── Button / switch (Attack trigger, active-low) ───────────────── */
#define BUTTON_PIN    2

/* ── State-machine timing (ms) ──────────────────────────────────── */
#define IDLE_FWD_MS              1000
#define IDLE_TURN_MS              300
#define ATTACK_FWD_MS             300
#define ATTACK_BACK_MS            500
#define FAST_SEARCH_TURN_MS       350
#define FAST_SEARCH_MAX_CYCLES      5

/* ── Motor duty values (0–255) ──────────────────────────────────── */
#define DUTY_IDLE_FWD       150
#define DUTY_DETECTED_FWD   200
#define DUTY_ATTACK_FWD     255
#define DUTY_ATTACK_BACK    100
#define DUTY_TURN           120
#define DUTY_FAST_TURN      140
#define DUTY_BORDER_TURN    180

/* ── Audio note → command frequency bands (Hz) ──────────────────── */
#define AUDIO_FWD_LO     400.0f
#define AUDIO_FWD_HI     600.0f
#define AUDIO_BACK_LO    600.0f
#define AUDIO_BACK_HI    800.0f
#define AUDIO_LEFT_LO    800.0f
#define AUDIO_LEFT_HI   1000.0f
#define AUDIO_RIGHT_LO  1000.0f
#define AUDIO_RIGHT_HI  1200.0f

#endif /* CONFIG_H */
