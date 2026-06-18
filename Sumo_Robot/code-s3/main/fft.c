#include "fft.h"
#include "config.h"
#include "esp_dsp.h"
#include <math.h>
#include <stdint.h>

static float fft_buffer[FFT_SIZE * 2];
static float hann_window[FFT_SIZE];
static uint8_t window_ready = 0;

void fft_compute(float *input, float *output) {
    /* Inicializar ventana Hann una sola vez */
    if (!window_ready) {
        dsps_wind_hann_f32(hann_window, FFT_SIZE);
        window_ready = 1;
    }

    /* Quitar DC (media) */
    float mean = 0.0f;
    for (int i = 0; i < FFT_SIZE; i++) mean += input[i];
    mean /= FFT_SIZE;

    /* Aplicar centrado, normalización 12-bit y ventana Hann */
    for (int i = 0; i < FFT_SIZE; i++) {
        float v = ((input[i] - mean) / 2048.0f) * hann_window[i];
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

/*
 * Devuelve frecuencia del pico en Hz y su magnitud en *out_magnitude.
 * Retorna 0.0 si la magnitud del pico está bajo AUDIO_MIN_MAGNITUDE
 * (indica que no hay señal real, solo ruido de fondo).
 */
float fft_find_peak(float *spectrum, float fs, float *out_magnitude) {
    // Ignorar todo bajo MIN_FREQ_HZ (ruido 50Hz y armónicos)
    const float MIN_FREQ_HZ = 200.0f;
    int start_bin = (int)(MIN_FREQ_HZ * FFT_SIZE / fs) + 1;  // ~51 con fs=4000, FFT=1024

    int   max_idx = start_bin;
    float max_val = spectrum[start_bin];
    int end_bin = (int)(FFT_SIZE / 2 * 0.9f);  // ~460 con FFT=1024

    for (int i = start_bin + 1; i < end_bin; i++) {
        if (spectrum[i] > max_val) {
            max_val = spectrum[i];
            max_idx = i;
        }
    }

    if (out_magnitude) *out_magnitude = max_val;

    if (max_val < AUDIO_MIN_MAGNITUDE) return 0.0f;

    return (max_idx * fs) / FFT_SIZE;
}