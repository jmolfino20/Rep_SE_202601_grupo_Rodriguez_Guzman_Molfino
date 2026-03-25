#include <stdio.h>
#include <stdlib.h>
#include "esp_heap_caps.h"
#include "esp_cpu.h"

#define VECTOR_SIZE 20  // Esto es para hacerlo como sale en el enunciado, todos son size=20

/* Memorias estaticas */
// DRAM estatica
DRAM_ATTR static int vector_dram_static[VECTOR_SIZE] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
DRAM_ATTR static int num_dram_static = 5;
DRAM_ATTR static int result_dram_static[VECTOR_SIZE];
// IRAM estatica
IRAM_ATTR static int vector_iram_static[VECTOR_SIZE] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
IRAM_ATTR static int num_iram_static = 5;
IRAM_ATTR static int result_iram_static[VECTOR_SIZE];
// RTC estatica
RTC_DATA_ATTR static int vector_rtc_static[VECTOR_SIZE] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
RTC_DATA_ATTR static int num_rtc_static = 5;
RTC_DATA_ATTR static int result_rtc_static[VECTOR_SIZE];

/* Este no se si es estático pero asi salia en el enunciado */
// FLASH (Read only)
const __attribute__((section(".rodata"))) int vector_flash_ext[VECTOR_SIZE] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
const __attribute__((section(".rodata"))) int num_flash = 5;
// result_flash va a estar abajo porque no puede ser un result only

// Funcion que sale en el enunciado
void multiply_vector_scalar(volatile int *vector, volatile int *result, int num, int size) {
    for (int i = 0; i < size; i++) {
        result[i] = vector[i] * num;
    }
}

// Para no hacer el for que sale, se hace esta funcion y se repite en todas las memorias
void test_memory(const char *name, volatile int *vector, volatile int *result, int num) {
    uint32_t start = esp_cpu_get_cycle_count();
    multiply_vector_scalar(vector, result, num, VECTOR_SIZE);
    uint32_t end = esp_cpu_get_cycle_count();

    uint32_t cycles = end - start;
    float cycles_per_byte = (float)cycles / (VECTOR_SIZE * sizeof(int));

    printf("%s: %lu cycles, %.4f cycles/byte\n", name, (unsigned long)cycles, cycles_per_byte);
}

void app_main() {
    /* Memorias dinamicas */
    int *vector_dram_dynamic = (int*)malloc(VECTOR_SIZE * sizeof(int));
    int *result_dram_dynamic = (int*)malloc(VECTOR_SIZE * sizeof(int));
    int num_dram_dynamic = 5;  // Esto no sale así en el pseudocodigo pero le pongo 5 porque sino no se como hacer que valga 5

    int *vector_iram_dynamic = (int*)heap_caps_malloc(VECTOR_SIZE * sizeof(int), MALLOC_CAP_EXEC);
    int *result_iram_dynamic = (int*)heap_caps_malloc(VECTOR_SIZE * sizeof(int), MALLOC_CAP_EXEC);
    int num_iram_dynamic = 5;

    // Mi ESP32 chino no tenía PSRAM, probemos con el del lab
    // int *vector_psram_dynamic = heap_caps_malloc(VECTOR_SIZE * sizeof(int), MALLOC_CAP_SPIRAM);
    // int *result_psram_dynamic = heap_caps_malloc(VECTOR_SIZE * sizeof(int), MALLOC_CAP_SPIRAM);
    // int num_psram_dynamic = 5;

    // Inicializar los valores de las dinamicas
    for (int i = 0; i < VECTOR_SIZE; i++) {
        vector_dram_dynamic[i] = i + 1;
        vector_iram_dynamic[i] = i + 1;
        // vector_psram_dynamic[i] = i + 1;
    }

    // Tests
    test_memory("DRAM static", vector_dram_static, result_dram_static, num_dram_static);
    test_memory("IRAM static", vector_iram_static, result_iram_static, num_iram_static);
    test_memory("RTC static", vector_rtc_static, result_rtc_static, num_rtc_static);

    int result_flash[VECTOR_SIZE];
    test_memory("FLASH (.rodata)", (int*)vector_flash_ext, result_flash, num_flash);

    test_memory("DRAM dynamic", vector_dram_dynamic, result_dram_dynamic, num_dram_dynamic);
    test_memory("IRAM dynamic", vector_iram_dynamic, result_iram_dynamic, num_iram_dynamic);
    // test_memory("PSRAM dynamic", vector_psram_dynamic, result_psram_dynamic, num_psram_dynamic);
}