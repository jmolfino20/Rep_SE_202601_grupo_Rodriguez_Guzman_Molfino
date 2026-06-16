#include "adc_audio.h"
#include "config.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_rom_sys.h"
#include "esp_check.h"

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
    /* ADC_BITWIDTH_12 = 12, so max = 4096, center at 2048 */
    float offset = (float)(1 << (int)ADC_BITWIDTH_CFG) / 2.0f;

    for (int i = 0; i < FFT_SIZE; i++) {
        adc_oneshot_read(adc_handle, AUDIO_ADC_CHANNEL, &raw);
        buffer[i] = (float)raw - offset;
        esp_rom_delay_us((uint32_t)(1000000.0f / SAMPLE_RATE));
    }
}
