#include "audio.h"
#include "globals.h"
#include "motor.h"
#include "adc_audio.h"
#include "fft.h"
#include "config.h"
#include "esp_dsp.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TAG "AUDIO"

typedef enum { DIR_NONE, DIR_FWD, DIR_BACK, DIR_LEFT, DIR_RIGHT } AudioDir;

static const char *dir_name(AudioDir d) {
    switch (d) {
        case DIR_FWD:   return "ADELANTE";
        case DIR_BACK:  return "ATRAS";
        case DIR_LEFT:  return "IZQUIERDA";
        case DIR_RIGHT: return "DERECHA";
        default:        return "sin nota";
    }
}

/* Calcula peak-to-peak del buffer (en cuentas ADC, pre-centrado) */
static float signal_pp(const float *buf, int n) {
    float mn = buf[0], mx = buf[0];
    for (int i = 1; i < n; i++) {
        if (buf[i] < mn) mn = buf[i];
        if (buf[i] > mx) mx = buf[i];
    }
    return mx - mn;
}

/*
 * Rutina de interrupción 1 – control por audio (Core 1)
 *
 * Bandas configuradas en config.h:
 *   AUDIO_FWD_LO  – AUDIO_FWD_HI   → adelante
 *   AUDIO_BACK_LO – AUDIO_BACK_HI  → atrás
 *   AUDIO_LEFT_LO – AUDIO_LEFT_HI  → izquierda
 *   AUDIO_RIGHT_LO– AUDIO_RIGHT_HI → derecha
 *
 * Si la magnitud del pico FFT < AUDIO_MIN_MAGNITUDE se considera
 * ruido y no se activa ningún comando.
 */
void audio_task(void *arg) {
    static float signal[FFT_SIZE];
    static float spectrum[FFT_SIZE / 2];

    ESP_ERROR_CHECK(dsps_fft2r_init_fc32(NULL, FFT_SIZE));
    adc_audio_init();

    /* Calibrar frecuencia de muestreo real */
    int64_t t0 = esp_timer_get_time();
    adc_audio_sample(signal);
    int64_t t1 = esp_timer_get_time();
    float real_fs = (float)FFT_SIZE / ((t1 - t0) / 1e6f);

    ESP_LOGI(TAG, "FS calibrada: %.1f Hz  |  resolucion FFT: %.2f Hz/bin",
             real_fs, real_fs / FFT_SIZE);
    ESP_LOGI(TAG, "Bandas: fwd=%.0f-%.0f  back=%.0f-%.0f  izq=%.0f-%.0f  der=%.0f-%.0f  (magnitud min=%.3f)",
             AUDIO_FWD_LO, AUDIO_FWD_HI,
             AUDIO_BACK_LO, AUDIO_BACK_HI,
             AUDIO_LEFT_LO, AUDIO_LEFT_HI,
             AUDIO_RIGHT_LO, AUDIO_RIGHT_HI,
             AUDIO_MIN_MAGNITUDE);
    ESP_LOGI(TAG, "DIAGNOSTICO: si 'ADC p-p' es siempre <20 cnts -> micro no conectado o sin alimentar");

    AudioDir prev_dir = DIR_NONE;

    while (1) {
        adc_audio_sample(signal);

        /* ── Diagnóstico de señal ADC ──────────────────────────────── */
        float pp = signal_pp(signal, FFT_SIZE);   /* peak-to-peak en cuentas */

        fft_compute(signal, spectrum);

        float magnitude = 0.0f;
        float freq = fft_find_peak(spectrum, real_fs, &magnitude);

        AudioDir dir = DIR_NONE;
        if (freq > 0.0f) {   /* 0 significa magnitud bajo umbral */
            if      (freq > AUDIO_FWD_LO   && freq < AUDIO_FWD_HI)   dir = DIR_FWD;
            else if (freq > AUDIO_BACK_LO  && freq < AUDIO_BACK_HI)  dir = DIR_BACK;
            else if (freq > AUDIO_LEFT_LO  && freq < AUDIO_LEFT_HI)  dir = DIR_LEFT;
            else if (freq > AUDIO_RIGHT_LO && freq < AUDIO_RIGHT_HI) dir = DIR_RIGHT;
        }

        /* ── Monitor continuo ──────────────────────────────────────── *
         * ADC p-p:  <20 cnts  → micro sin señal (hardware issue)      *
         *           20-200    → señal débil (alejar fuente de ruido)   *
         *           >200      → señal fuerte (normal)                  *
         * mag:      <MIN_MAG  → descartado como ruido                  */
        if (dir != DIR_NONE) {
            ESP_LOGW(TAG, "MONITOR | ADC p-p=%5.0f cnts | freq=%6.1f Hz | mag=%.4f | -> %-10s  MOTOR ON",
                     pp, freq, magnitude, dir_name(dir));
        } else if (freq > 0.0f) {
            ESP_LOGI(TAG, "MONITOR | ADC p-p=%5.0f cnts | freq=%6.1f Hz | mag=%.4f | fuera de banda",
                     pp, freq, magnitude);
        } else {
            ESP_LOGI(TAG, "MONITOR | ADC p-p=%5.0f cnts | freq=  ---   Hz | mag=%.4f | bajo umbral (ruido)",
                     pp, magnitude);
        }

        /* ── Accionar motor ────────────────────────────────────────── */
        if (dir != DIR_NONE) {
            if (dir != prev_dir) {
                ESP_LOGW(TAG, "INT AUDIO: nueva direccion -> %s", dir_name(dir));
            }
            g_audio_override = 1;
            switch (dir) {
                case DIR_FWD:   motor_forward (DUTY_DETECTED_FWD, DUTY_DETECTED_FWD); break;
                case DIR_BACK:  motor_backward(DUTY_DETECTED_FWD, DUTY_DETECTED_FWD); break;
                case DIR_LEFT:  motor_left    (DUTY_TURN, DUTY_TURN);                  break;
                case DIR_RIGHT: motor_right   (DUTY_TURN, DUTY_TURN);                  break;
                default: break;
            }
        } else {
            if (g_audio_override) {
                ESP_LOGW(TAG, "INT AUDIO: sin nota -> motor libre");
                motor_stop();
            }
            g_audio_override = 0;
        }

        prev_dir = dir;
        vTaskDelay(1);   /* 1 tick (~10 ms) — cede CPU para que IDLE1 limpie el watchdog */
    }
}
