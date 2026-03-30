#include "adc_audio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_rom_sys.h"
#include "esp_check.h"
#include "config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Conectar Out del microfono a GPIO1 (ADC0) del ESP32-S3
#define ADC_CHANNEL ADC_CHANNEL_0
#define ADC_UNIT ADC_UNIT_1

static adc_oneshot_unit_handle_t adc_handle;

void adc_audio_init() {
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));

    adc_oneshot_chan_cfg_t chan_config = {
        .bitwidth = ADC_BITWIDTH_CONFIG,   // resolucion de 12 bits
        .atten = ADC_ATTEN_DB_12,      // atenuacion
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, ADC_CHANNEL, &chan_config));
}

void adc_audio_sample(float *buffer) {
    int raw;

    for (int i = 0; i < FFT_SIZE; i++) {
        adc_oneshot_read(adc_handle, ADC_CHANNEL, &raw);     // se lee (pero va de 0 a 4095)

        float offset = (1 << (ADC_BITWIDTH_CONFIG)) / 2.0;   // (se calcula la mitad)
        buffer[i] = (float)raw - offset;                     // se centra para que vaya de (-2048 a 2048)

        esp_rom_delay_us((uint32_t)SAMPLE_PERIOD_US);        // se hace un delay segun la frecuencia de muestreo
    }
}
