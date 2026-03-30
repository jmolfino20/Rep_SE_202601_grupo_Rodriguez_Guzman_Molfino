#include <stdio.h>
#include "adc_audio.h"
#include "fft.h"
#include "motor.h"
#include "esp_log.h"
#include "esp_dsp.h"
#include "config.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


static float real_fs = SAMPLE_RATE;
static float signal[FFT_SIZE];
static float spectrum[FFT_SIZE/2];

void app_main() {
    // Se inicializa lo que se va a usar
    ESP_ERROR_CHECK(dsps_fft2r_init_fc32(NULL, FFT_SIZE));   // FFT
    adc_audio_init();   // ADC
    motor_init();   // Puente H
    
    // Este menjunje es pa calcular el tiempo que se demora en leer y así se puede tener la freq real
    int64_t start = esp_timer_get_time();
    adc_audio_sample(signal);
    int64_t end = esp_timer_get_time();
    real_fs = FFT_SIZE / ((end - start) / 1e6);
    
    while (1) {
        vTaskDelay(1);   // Hay que hacer que a veces se ceda la tarea porque sino estaba dando un error

        adc_audio_sample(signal);

        fft_compute(signal, spectrum);
        float freq = fft_find_peak(spectrum, real_fs);

        ESP_LOGI("MAIN", "Freq: %.2f Hz", freq);   // Se imprime en consola

        if (freq > 400 && freq < 600) move_forward();
        else if (freq > 600 && freq < 800) move_backward();
        else if (freq > 800 && freq < 1000) move_left();
        else if (freq > 1000 && freq < 1200) move_right();
        else stop();
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
