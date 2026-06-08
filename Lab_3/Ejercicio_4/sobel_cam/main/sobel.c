#include "sobel.h"
#include <stdlib.h>

#define SOBEL_THRESH  70
#define BORDE_RATIO   0.10f

static uint8_t sobel_pixel(uint8_t *img, int x, int y, int w, int h) {
    if (x < 1 || x >= w-1 || y < 1 || y >= h-1) return 0;
    int gx = -img[(y-1)*w+(x-1)] - 2*img[y*w+(x-1)] - img[(y+1)*w+(x-1)] + img[(y-1)*w+(x+1)] + 2*img[y*w+(x+1)] + img[(y+1)*w+(x+1)];
    int gy = -img[(y-1)*w+(x-1)] - 2*img[(y-1)*w+x] - img[(y-1)*w+(x+1)] + img[(y+1)*w+(x-1)] + 2*img[(y+1)*w+x] + img[(y+1)*w+(x+1)];
    int mag = abs(gx) + abs(gy);
    return (uint8_t)(mag > 255 ? 255 : mag);
}

BordeZonas sobel_detectar(uint8_t *img, int w, int h) {
    int col1 = w / 3;
    int col2 = 2 * w / 3;
    int fila1 = h / 3;
    int fila2 = 2 * h / 3;

    int conteos[9] = {0};
    int totales[9] = {0};

    for (int y = 1; y < h-1; y++) {
        for (int x = 1; x < w-1; x++) {
            uint8_t mag = sobel_pixel(img, x, y, w, h);
            int zona_col = (x < col1) ? 0 : (x < col2 ? 1 : 2);
            int zona_fil = (y < fila1) ? 0 : (y < fila2 ? 1 : 2);
            int zona = zona_fil * 3 + zona_col;
            totales[zona]++;
            if (mag > SOBEL_THRESH) conteos[zona]++;
        }
    }

    BordeZonas z = {0};

    z.sup_izq = (float)conteos[0]/totales[0] > BORDE_RATIO;
    z.sup_cen = (float)conteos[1]/totales[1] > BORDE_RATIO;
    z.sup_der = (float)conteos[2]/totales[2] > BORDE_RATIO;
    z.mid_izq = (float)conteos[3]/totales[3] > BORDE_RATIO;
    z.mid_cen = (float)conteos[4]/totales[4] > BORDE_RATIO;
    z.mid_der = (float)conteos[5]/totales[5] > BORDE_RATIO;
    z.inf_izq = (float)conteos[6]/totales[6] > BORDE_RATIO;
    z.inf_cen = (float)conteos[7]/totales[7] > BORDE_RATIO;
    z.inf_der = (float)conteos[8]/totales[8] > BORDE_RATIO;
    return z;
}

NavCmd sobel_decidir(BordeZonas z) {

    //if (z.inf_izq && z.mid_izq) return CMD_LEFT;
    if (z.sup_cen || z.sup_der) return CMD_RIGHT;
    if (z.inf_cen || z.mid_cen || z.sup_cen) return CMD_RIGHT;
    //if (z.inf_der || z.mid_der) return CMD_LEFT;
    if (z.inf_izq && z.mid_izq && z.sup_izq) return CMD_FORWARD;
    if (z.inf_izq || z.mid_izq || z.sup_izq) return CMD_FORWARD;

    return CMD_FORWARD;  // era CMD_LEFT
}