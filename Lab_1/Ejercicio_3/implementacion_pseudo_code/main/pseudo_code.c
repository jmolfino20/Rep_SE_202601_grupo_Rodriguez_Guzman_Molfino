#include <stdio.h>
#include <inttypes.h>
#include "xtensa/core-macros.h"
#include "esp_timer.h"
#include "sdkconfig.h"

// Variables globales
uint32_t start_cycles, end_cycles;
int64_t start_time, end_time;

uint32_t instruction_count = 0;
// Inicialización
void init_time() {
    start_time = esp_timer_get_time();
}

void end_time_func() {
    end_time = esp_timer_get_time();
}

void init_cycle_count() {
    start_cycles = XTHAL_GET_CCOUNT();
}

void end_cycle_count() {
    end_cycles = XTHAL_GET_CCOUNT();
}


void init_instruction_count() {
    instruction_count = 0;
}

// Cálculos
void calculate_Time() {
    double time_us = (double)(end_time - start_time);
    printf("Tiempo: %.2f us\n", time_us);
}

void calculate_Cycles(uint32_t cycles) {
    printf("Ciclos (netos): %" PRIu32 "\n", cycles);
}

void calculate_CPI(uint32_t cycles) {
    if (instruction_count == 0) return;

    double cpi = (double)cycles / instruction_count;
    printf("CPI (ajustado): %.4f\n", cpi);
}

void calculate_time_using_cycles(uint32_t cycles) {
    uint32_t freq_hz = CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ * 1000000;
    double time_sec = (double)cycles / freq_hz;
    printf("Tiempo usando ciclos (neto): %e s\n", time_sec);
    //printf("Frecuencia del CPU: %ld MHz\n", freq_hz / 1000000);
}

// Check
void check_results(int pass) {
    if (pass)
        printf("PASS\n");
    else
        printf("FAIL\n");
}

void app_main() {

    uint32_t overhead_cycles;
    uint32_t total_cycles;
    uint32_t net_cycles;

    volatile int var_1 = 233;
    volatile int var_2 = 128;
    volatile int result_0, result_1, result_2, result_3;
    volatile float result_4;
    int X = 100000;
    
    init_cycle_count();
    for (int i = 0; i < X; i++) {}
    end_cycle_count();
    overhead_cycles = end_cycles - start_cycles;


    init_time();
    init_instruction_count();
    init_cycle_count();

    instruction_count = X * 5; // 5 operaciones por iteración
    for (int i = 0; i < X; i++) {
        result_0 = var_1 + var_2;
        result_1 = var_1 + 10;
        result_2 = var_1 % var_2;
        result_3 = var_1 * var_2;
        result_4 = (float)var_1 / var_2;
    }

    end_cycle_count();
    end_time_func();

    total_cycles = end_cycles - start_cycles;
    net_cycles = total_cycles - overhead_cycles;
    int pass = (result_0 == 361 && result_1 == 243 && result_2 == 105 && result_3 == 29824 && result_4 == 1.8203125);
    check_results(pass);

    calculate_Time();
    calculate_Cycles(net_cycles);
    calculate_CPI(net_cycles);
    calculate_time_using_cycles(net_cycles);
}