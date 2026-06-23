#include "adc_audio.h"
#include "config.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_rom_sys.h"
#include "esp_check.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static adc_oneshot_unit_handle_t adc_handle;

void adc_audio_init(void) {
    adc_oneshot_unit_init_cfg_t init_cfg = {
        .unit_id  = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_cfg, &adc_handle));

    adc_oneshot_chan_cfg_t chan_cfg = {
        .bitwidth = ADC_BITWIDTH_CFG,
        .atten    = ADC_ATTEN_DB_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, AUDIO_ADC_CHANNEL, &chan_cfg));
}

void adc_audio_sample(float *buffer) {
    int raw;
    const float offset    = 2048.0f;
    const int64_t period_us = (int64_t)(1000000.0f / SAMPLE_RATE);
    int64_t deadline = esp_timer_get_time();

    for (int i = 0; i < FFT_SIZE; i++) {
        deadline += period_us;
        adc_oneshot_read(adc_handle, AUDIO_ADC_CHANNEL, &raw);
        buffer[i] = (float)raw - offset;

        /* Cada 256 muestras cede CPU 1 tick para que IDLE1 pueda alimentar el TWDT */
        if ((i & 0xFF) == 0xFF) {
            vTaskDelay(pdMS_TO_TICKS(1));
            deadline = esp_timer_get_time();   /* recalibrar tras el yield */
        } else {
            int64_t now = esp_timer_get_time();
            if (deadline > now)
                esp_rom_delay_us((uint32_t)(deadline - now));
        }
    }
}