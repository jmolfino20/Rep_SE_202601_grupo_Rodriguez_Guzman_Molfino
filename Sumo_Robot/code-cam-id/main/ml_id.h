#ifndef ML_ID_H
#define ML_ID_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void ml_id_init(void);
bool ml_id_detect(uint8_t *img, int w, int h);

#ifdef __cplusplus
}
#endif

#endif
