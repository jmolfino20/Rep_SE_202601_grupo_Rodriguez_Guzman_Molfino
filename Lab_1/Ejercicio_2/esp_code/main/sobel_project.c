#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "pikachu_array.h"

static uint8_t output[96][96];

void app_main(void)
{
    // Sobel
    for (int i = 1; i < 95; i++) {
        for (int j = 1; j < 95; j++) {

            int gx =
                -image[i-1][j-1] + image[i-1][j+1]
                -2*image[i][j-1] + 2*image[i][j+1]
                -image[i+1][j-1] + image[i+1][j+1];

            int gy =
                -image[i-1][j-1] -2*image[i-1][j] -image[i-1][j+1]
                +image[i+1][j-1] +2*image[i+1][j] +image[i+1][j+1];

            int magnitude = abs(gx) + abs(gy);

            if (magnitude > 255) magnitude = 255;

            output[i][j] = magnitude;
        }
    }

    // Print serial
    for (int i = 0; i < 96; i++) {
        printf("[");
        for (int j = 0; j < 96; j++) {
            printf("%d", output[i][j]);
            if (j < 95) printf(",");
        }
        printf("]\n");
        fflush(stdout);
    }
}