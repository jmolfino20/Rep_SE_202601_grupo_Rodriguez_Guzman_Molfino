#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void ml_id_init(void);

/* Recibe frame QVGA (320x240 grayscale). Devuelve 0=Sin ID, 1=Con ID, -1=error */
int ml_id_detect(uint8_t *img, int w, int h);

#ifdef __cplusplus
}
#endif