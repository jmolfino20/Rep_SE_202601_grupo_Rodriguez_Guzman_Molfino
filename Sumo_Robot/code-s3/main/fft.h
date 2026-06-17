#ifndef FFT_H
#define FFT_H

/*
 * fft_compute: aplica ventana Hann + FFT sobre input[FFT_SIZE].
 *              Escribe magnitudes en output[FFT_SIZE/2].
 *
 * fft_find_peak: devuelve la frecuencia del pico dominante (Hz).
 *                Escribe la magnitud del pico en *out_magnitude (si no es NULL).
 *                Retorna 0.0 si la magnitud es inferior a AUDIO_MIN_MAGNITUDE.
 */
void  fft_compute(float *input, float *output);
float fft_find_peak(float *spectrum, float fs, float *out_magnitude);

#endif
