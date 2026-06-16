#include "audio.h"
#include "globals.h"
#include "motor.h"
#include "adc_audio.h"
#include "fft.h"
#include "config.h"
#include "esp_dsp.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/*
 * Rutina de interrupción 1 – control por audio (Core 1)
 *
 * Bandas de frecuencia (Hz) → movimiento:
 *   400–600   adelante
 *   600–800   atrás
 *   800–1000  izquierda
 *   1000–1200 derecha
 *   otro      sin override
 *
 * Cuando g_audio_override = 1, state_machine_task cede el motor.
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

    while (1) {
        adc_audio_sample(signal);
        fft_compute(signal, spectrum);
        float freq = fft_find_peak(spectrum, real_fs);

        if (freq > AUDIO_FWD_LO && freq < AUDIO_FWD_HI) {
            g_audio_override = 1;
            motor_forward(DUTY_DETECTED_FWD, DUTY_DETECTED_FWD);
        } else if (freq > AUDIO_BACK_LO && freq < AUDIO_BACK_HI) {
            g_audio_override = 1;
            motor_backward(DUTY_DETECTED_FWD, DUTY_DETECTED_FWD);
        } else if (freq > AUDIO_LEFT_LO && freq < AUDIO_LEFT_HI) {
            g_audio_override = 1;
            motor_left(DUTY_TURN, DUTY_TURN);
        } else if (freq > AUDIO_RIGHT_LO && freq < AUDIO_RIGHT_HI) {
            g_audio_override = 1;
            motor_right(DUTY_TURN, DUTY_TURN);
        } else {
            g_audio_override = 0;
        }

        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
