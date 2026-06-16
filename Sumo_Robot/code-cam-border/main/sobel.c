#include "sobel.h"
#include "config.h"
#include <stdlib.h>

static uint8_t sobel_pixel(const uint8_t *img, int x, int y, int w, int h) {
    if (x < 1 || x >= w-1 || y < 1 || y >= h-1) return 0;
    int gx = -img[(y-1)*w+(x-1)] - 2*img[y*w+(x-1)] - img[(y+1)*w+(x-1)]
             +img[(y-1)*w+(x+1)] + 2*img[y*w+(x+1)] + img[(y+1)*w+(x+1)];
    int gy = -img[(y-1)*w+(x-1)] - 2*img[(y-1)*w+x] - img[(y-1)*w+(x+1)]
             +img[(y+1)*w+(x-1)] + 2*img[(y+1)*w+x] + img[(y+1)*w+(x+1)];
    int mag = abs(gx) + abs(gy);
    return (uint8_t)(mag > 255 ? 255 : mag);
}

/*
 * Returns true when the image contains border-level edge density in the
 * central or right zones (matches the CMD_RIGHT conditions from Lab3/Ej4).
 *
 * Zone layout (3x3 grid):
 *   0(sup_izq) 1(sup_cen) 2(sup_der)
 *   3(mid_izq) 4(mid_cen) 5(mid_der)
 *   6(inf_izq) 7(inf_cen) 8(inf_der)
 */
bool sobel_border_detected(uint8_t *img, int w, int h) {
    int col1 = w / 3, col2 = 2 * w / 3;
    int row1 = h / 3, row2 = 2 * h / 3;
    int counts[9] = {0}, totals[9] = {0};

    for (int y = 1; y < h-1; y++) {
        for (int x = 1; x < w-1; x++) {
            uint8_t mag = sobel_pixel(img, x, y, w, h);
            int zc = (x < col1) ? 0 : (x < col2 ? 1 : 2);
            int zr = (y < row1) ? 0 : (y < row2 ? 1 : 2);
            int z  = zr * 3 + zc;
            totals[z]++;
            if (mag > SOBEL_THRESH) counts[z]++;
        }
    }

    /* Check central and right zones: 1(sup_cen) 2(sup_der)
     *                                4(mid_cen) 5(mid_der)
     *                                7(inf_cen) 8(inf_der) */
    static const int key_zones[] = {1, 2, 4, 5, 7, 8};
    for (int i = 0; i < 6; i++) {
        int z = key_zones[i];
        if (totals[z] > 0 && (float)counts[z] / totals[z] > BORDER_RATIO)
            return true;
    }
    return false;
}
