#ifndef CONFIG_H
#define CONFIG_H

// Frecuencia de muestreo (Hz)
// (500.0, 1000.0, 2000.0, 4000.0)
#define SAMPLE_RATE 4000.0

// Tamaño de FFT
// (128, 256, 512, 1024)
#define FFT_SIZE 1024

// Resolución ADC
// (ADC_BITWIDTH_9, ADC_BITWIDTH_10, ADC_BITWIDTH_11, ADC_BITWIDTH_12)
#define ADC_BITWIDTH_CONFIG ADC_BITWIDTH_12

// Delay entre muestras (us)
#define SAMPLE_PERIOD_US (1000000.0 / SAMPLE_RATE)

#endif
