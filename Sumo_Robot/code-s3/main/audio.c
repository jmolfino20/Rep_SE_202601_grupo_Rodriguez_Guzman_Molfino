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
#include <math.h>

#define TAG "AUDIO"

typedef enum {
    DIR_NONE,
    DIR_FWD,
    DIR_BACK,
    DIR_LEFT,
    DIR_RIGHT,
    DIR_ATTACK,
} AudioDir;

static const char *dir_name(AudioDir d) {
    switch (d) {
        case DIR_FWD:    return "ADELANTE";
        case DIR_BACK:   return "ATRAS";
        case DIR_LEFT:   return "IZQUIERDA";
        case DIR_RIGHT:  return "DERECHA";
        case DIR_ATTACK: return "ATAQUE";
        default:         return "sin nota";
    }
}

static float signal_pp(const float *buf, int n) {
    float mn = buf[0], mx = buf[0];
    for (int i = 1; i < n; i++) {
        if (buf[i] < mn) mn = buf[i];
        if (buf[i] > mx) mx = buf[i];
    }
    return mx - mn;
}

/* Busca el pico mas fuerte dentro de [f_lo, f_hi] Hz */
static float fft_peak_in_band(const float *spectrum, float real_fs,
                               float f_lo, float f_hi, float *out_mag) {
    float bin_hz  = real_fs / FFT_SIZE;
    int   bin_lo  = (int)(f_lo / bin_hz);
    int   bin_hi  = (int)(f_hi / bin_hz) + 1;
    int   n       = FFT_SIZE / 2;

    if (bin_lo < 1)  bin_lo = 1;
    if (bin_hi >= n) bin_hi = n - 1;

    float best_mag = 0.0f;
    int   best_bin = -1;

    for (int i = bin_lo; i <= bin_hi; i++) {
        if (spectrum[i] > best_mag) {
            best_mag = spectrum[i];
            best_bin = i;
        }
    }

    *out_mag = best_mag;
    return (best_bin >= 0) ? best_bin * bin_hz : 0.0f;
}

#define DTMF_ROW_LO   650.0f
#define DTMF_ROW_HI   900.0f
#define DTMF_COL_LO  1150.0f
#define DTMF_COL_HI  1700.0f

static inline bool near_freq(float f, float target) {
    return fabsf(f - target) <= AUDIO_TOLERANCE;
}

static AudioDir classify_dtmf(float fa, float fb) {
    float lo = fa < fb ? fa : fb;
    float hi = fa < fb ? fb : fa;

    if (near_freq(lo, AUDIO_1_F1) && near_freq(hi, AUDIO_1_F2)) return DIR_FWD;
    if (near_freq(lo, AUDIO_4_F1) && near_freq(hi, AUDIO_4_F2)) return DIR_LEFT;
    if (near_freq(lo, AUDIO_6_F1) && near_freq(hi, AUDIO_6_F2)) return DIR_RIGHT;
    if (near_freq(lo, AUDIO_9_F1) && near_freq(hi, AUDIO_9_F2)) return DIR_BACK;
    if (near_freq(lo, AUDIO_A_F1) && near_freq(hi, AUDIO_A_F2)) return DIR_ATTACK;

    return DIR_NONE;
}

void audio_task(void *arg) {
    static float signal[FFT_SIZE];
    static float spectrum[FFT_SIZE / 2];

    ESP_ERROR_CHECK(dsps_fft2r_init_fc32(NULL, FFT_SIZE));
    adc_audio_init();

    int64_t t0 = esp_timer_get_time();
    adc_audio_sample(signal);
    int64_t t1 = esp_timer_get_time();
    float real_fs = (float)FFT_SIZE / ((t1 - t0) / 1e6f);

    float bin_hz = real_fs / FFT_SIZE;

    ESP_LOGI(TAG, "FS calibrada: %.1f Hz  |  resolucion FFT: %.2f Hz/bin", real_fs, bin_hz);
    ESP_LOGI(TAG, "Tolerancia: +/- %.0f Hz  |  confirmacion: %d de %d frames",
             AUDIO_TOLERANCE, AUDIO_CONFIRM_NEEDED, AUDIO_CONFIRM_WINDOW);
    ESP_LOGI(TAG, "DTMF: FWD=%.0f+%.0f  BACK=%.0f+%.0f  IZQ=%.0f+%.0f  DER=%.0f+%.0f  ATK=%.0f+%.0f",
             AUDIO_1_F1, AUDIO_1_F2,
             AUDIO_9_F1, AUDIO_9_F2,
             AUDIO_4_F1, AUDIO_4_F2,
             AUDIO_6_F1, AUDIO_6_F2,
             AUDIO_A_F1, AUDIO_A_F2);

    AudioDir stable_dir   = DIR_NONE;
    AudioDir history[AUDIO_CONFIRM_WINDOW];
    int hist_idx = 0;
    for (int i = 0; i < AUDIO_CONFIRM_WINDOW; i++) history[i] = DIR_NONE;

    while (1) {
        adc_audio_sample(signal);

        float pp = signal_pp(signal, FFT_SIZE);

        fft_compute(signal, spectrum);

        float mag_row, mag_col;
        float f_row = fft_peak_in_band(spectrum, real_fs, DTMF_ROW_LO, DTMF_ROW_HI, &mag_row);
        float f_col = fft_peak_in_band(spectrum, real_fs, DTMF_COL_LO, DTMF_COL_HI, &mag_col);

        AudioDir raw_dir = DIR_NONE;
        if (mag_row >= AUDIO_MIN_MAGNITUDE && mag_col >= AUDIO_MIN_MAGNITUDE) {
            raw_dir = classify_dtmf(f_row, f_col);
        }

        history[hist_idx] = raw_dir;
        hist_idx = (hist_idx + 1) % AUDIO_CONFIRM_WINDOW;

        AudioDir detected = DIR_NONE;
        for (int d = DIR_FWD; d <= DIR_ATTACK; d++) {
            int count = 0;
            for (int i = 0; i < AUDIO_CONFIRM_WINDOW; i++) {
                if (history[i] == (AudioDir)d) count++;
            }
            if (count >= AUDIO_CONFIRM_NEEDED) {
                detected = (AudioDir)d;
                break;
            }
        }
        stable_dir = detected;

        if (stable_dir != DIR_NONE) {
            ESP_LOGW(TAG, "MONITOR | ADC p-p=%5.0f | row=%6.1f Hz mag=%.4f | col=%6.1f Hz mag=%.4f | -> %-10s  MOTOR ON",
                     pp, f_row, mag_row, f_col, mag_col, dir_name(stable_dir));
        } else if (raw_dir != DIR_NONE) {
            ESP_LOGI(TAG, "MONITOR | ADC p-p=%5.0f | row=%6.1f Hz | col=%6.1f Hz | candidato %-10s",
                     pp, f_row, f_col, dir_name(raw_dir));
        } else {
            ESP_LOGI(TAG, "MONITOR | ADC p-p=%5.0f | row=%6.1f Hz mag=%.4f | col=%6.1f Hz mag=%.4f | sin clasificar",
                     pp, f_row, mag_row, f_col, mag_col);
        }

        if (stable_dir != DIR_NONE && !g_border_detected) {
            g_audio_override = 1;
            switch (stable_dir) {
                case DIR_FWD:    motor_forward (DUTY_DETECTED_FWD, DUTY_DETECTED_FWD); break;
                case DIR_BACK:   motor_backward(DUTY_DETECTED_FWD, DUTY_DETECTED_FWD); break;
                case DIR_LEFT:   motor_left    (DUTY_TURN,         DUTY_TURN);          break;
                case DIR_RIGHT:  motor_right   (DUTY_TURN,         DUTY_TURN);          break;
                case DIR_ATTACK: motor_forward (DUTY_ATTACK_FWD,   DUTY_ATTACK_FWD);    break;
                default: break;
            }
        } else {
            if (g_audio_override) {
                ESP_LOGW(TAG, "INT AUDIO: %s -> motor libre",
                         g_border_detected ? "borde prioridad" : "sin nota");
                motor_stop();
            }
            g_audio_override = 0;
        }

        vTaskDelay(1);
    }
}
