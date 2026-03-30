#include "fft.h"
#include "esp_dsp.h"
#include <math.h>
#include "config.h"


static float fft_buffer[FFT_SIZE * 2];

void fft_compute(float *input, float *output) {
    for (int i = 0; i < FFT_SIZE; i++) {
        fft_buffer[2*i] = input[i];
        fft_buffer[2*i + 1] = 0;
    }

    dsps_fft2r_fc32(fft_buffer, FFT_SIZE);
    dsps_bit_rev_fc32(fft_buffer, FFT_SIZE);
    dsps_cplx2reC_fc32(fft_buffer, FFT_SIZE);

    for (int i = 0; i < FFT_SIZE/2; i++) {
        float real = fft_buffer[2*i];
        float imag = fft_buffer[2*i + 1];
        output[i] = sqrtf(real*real + imag*imag);
    }
}

float fft_find_peak(float *fft_output, float fs) {
    int max_index = 1;
    float max_value = fft_output[1];

    for (int i = 2; i < FFT_SIZE/2; i++) {
        if (fft_output[i] > max_value) {
            max_value = fft_output[i];
            max_index = i;
        }
    }

    return (max_index * fs) / FFT_SIZE;
}
