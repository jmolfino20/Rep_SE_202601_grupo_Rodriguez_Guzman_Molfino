#ifndef FFT_H
#define FFT_H

void fft_compute(float *input, float *output);
float fft_find_peak(float *fft_output, float fs);

#endif
