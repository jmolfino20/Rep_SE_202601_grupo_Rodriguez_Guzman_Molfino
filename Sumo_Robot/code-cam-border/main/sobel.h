#ifndef SOBEL_H
#define SOBEL_H

#include <stdint.h>
#include <stdbool.h>

bool sobel_border_detected(uint8_t *img, int w, int h);

#endif
