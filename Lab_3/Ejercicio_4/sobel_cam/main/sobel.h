#ifndef SOBEL_H
#define SOBEL_H

#include <stdint.h>

// Zonas de detección
typedef struct {
    uint8_t inf_izq;
    uint8_t inf_cen;
    uint8_t inf_der;
    uint8_t mid_izq;
    uint8_t mid_cen;
    uint8_t mid_der;
    uint8_t sup_izq;
    uint8_t sup_cen;
    uint8_t sup_der;
} BordeZonas;

// Comandos de navegación
typedef enum {
    CMD_STOP     = 0,
    CMD_FORWARD  = 3,
    CMD_LEFT     = 1,
    CMD_RIGHT    = 5,
    CMD_HARD_RIGHT = 9,
} NavCmd;

BordeZonas sobel_detectar(uint8_t *img, int w, int h);
NavCmd     sobel_decidir(BordeZonas z);

#endif