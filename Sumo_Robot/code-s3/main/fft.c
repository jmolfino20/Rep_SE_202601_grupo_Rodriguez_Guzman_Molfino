#include "fft.h"
#include "config.h"
#include "esp_dsp.h"
#include <math.h>

static float fft_buffer[FFT_SIZE * 2];

void fft_compute(float *input, float *output) {
    float mean = 0;
    for (int i = 0; i < FFT_SIZE; i++) mean += input[i];
    mean /= FFT_SIZE;

    /* Remove DC offset and normalize to ±1.0 (12-bit ADC range: ±2048) */
    for (int i = 0; i < FFT_SIZE; i++) {
        float v = (input[i] - mean) / 2048.0f;
        fft_buffer[2*i]     = v;
        fft_buffer[2*i + 1] = 0.0f;
    }

    dsps_fft2r_fc32(fft_buffer, FFT_SIZE);
    dsps_bit_rev_fc32(fft_buffer, FFT_SIZE);
    dsps_cplx2reC_fc32(fft_buffer, FFT_SIZE);

    for (int i = 0; i < FFT_SIZE / 2; i++) {
        float re = fft_buffer[2*i];
        float im = fft_buffer[2*i + 1];
        output[i] = sqrtf(re*re + im*im);
    }
}

float fft_find_peak(float *spectrum, float fs) {
    int   max_idx = 1;
    float max_val = spectrum[1];
    for (int i = 2; i < FFT_SIZE / 2; i++) {
        if (spectrum[i] > max_val) {
            max_val = spectrum[i];
            max_idx = i;
        }
    }
    return (max_idx * fs) / FFT_SIZE;
}
