#include "state_machine.h"
#include "globals.h"
#include "motor.h"
#include "config.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

typedef enum {
    STATE_IDLE,
    STATE_DETECTED,
    STATE_FAST_SEARCH,
    STATE_ATTACK,
} RobotState;

static inline bool button_pressed(void) {
    return gpio_get_level(BUTTON_PIN) == 0;   /* active-low */
}

/*
 * Waits ms milliseconds in 10 ms increments.
 * Returns false immediately if border is detected (preempt).
 */
static bool delay_ms(uint32_t ms) {
    for (uint32_t elapsed = 0; elapsed < ms; elapsed += 10) {
        if (g_border_detected) return false;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    return true;
}

/* ── States ──────────────────────────────────────────────────────── */

static RobotState run_idle(void) {
    /* Avanza recto */
    motor_forward(DUTY_IDLE_FWD, DUTY_IDLE_FWD);
    if (!delay_ms(IDLE_FWD_MS)) return STATE_IDLE;
    if (g_id_detected)    return STATE_DETECTED;
    if (button_pressed()) return STATE_ATTACK;

    /* Gira un poco a la derecha */
    motor_right(DUTY_TURN, DUTY_TURN);
    if (!delay_ms(IDLE_TURN_MS)) return STATE_IDLE;
    if (g_id_detected)    return STATE_DETECTED;
    if (button_pressed()) return STATE_ATTACK;

    return STATE_IDLE;
}

static RobotState run_detected(void) {
    /* Avanza hacia el oponente */
    motor_forward(DUTY_DETECTED_FWD, DUTY_DETECTED_FWD);
    if (!delay_ms(50)) return STATE_IDLE;

    if (button_pressed())  return STATE_ATTACK;
    if (!g_id_detected)    return STATE_FAST_SEARCH;

    return STATE_DETECTED;
}

static RobotState run_fast_search(void) {
    /* Primero busca girando a la izquierda (3 pequeños giros) */
    for (int i = 0; i < 3; i++) {
        motor_left(DUTY_FAST_TURN, DUTY_FAST_TURN);
        if (!delay_ms(FAST_SEARCH_TURN_MS)) return STATE_IDLE;
        if (g_id_detected)    return STATE_DETECTED;
        if (button_pressed()) return STATE_ATTACK;
    }

    /* Luego barre a la derecha */
    for (int i = 0; i < FAST_SEARCH_MAX_CYCLES; i++) {
        motor_right(DUTY_FAST_TURN, DUTY_FAST_TURN);
        if (!delay_ms(FAST_SEARCH_TURN_MS)) return STATE_IDLE;
        if (g_id_detected)    return STATE_DETECTED;
        if (button_pressed()) return STATE_ATTACK;
    }

    return STATE_IDLE;
}

static RobotState run_attack(void) {
    /* Empuje rápido */
    motor_forward(DUTY_ATTACK_FWD, DUTY_ATTACK_FWD);
    vTaskDelay(pdMS_TO_TICKS(ATTACK_FWD_MS));

    /* Retroceso lento */
    motor_backward(DUTY_ATTACK_BACK, DUTY_ATTACK_BACK);
    vTaskDelay(pdMS_TO_TICKS(ATTACK_BACK_MS));

    motor_stop();
    vTaskDelay(pdMS_TO_TICKS(100));

    return g_id_detected ? STATE_DETECTED : STATE_IDLE;
}

/* ── Task principal (Core 0) ─────────────────────────────────────── */

void state_machine_task(void *arg) {
    RobotState state = STATE_IDLE;

    while (1) {
        /*
         * Rutina de interrupción 2 (borde):
         * Mientras la CamBorder detecte borde, gira a la derecha.
         * Al terminar pasa a IDLE.
         */
        while (g_border_detected) {
            if (!g_audio_override)
                motor_right(DUTY_BORDER_TURN, DUTY_BORDER_TURN);
            vTaskDelay(pdMS_TO_TICKS(10));
        }

        /*
         * Rutina de interrupción 1 (audio):
         * Si el audio_task detectó una nota, cede el control del motor
         * y espera a que el audio termine.
         */
        if (g_audio_override) {
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }

        switch (state) {
            case STATE_IDLE:        state = run_idle();        break;
            case STATE_DETECTED:    state = run_detected();    break;
            case STATE_FAST_SEARCH: state = run_fast_search(); break;
            case STATE_ATTACK:      state = run_attack();      break;
            default:                state = STATE_IDLE;        break;
        }
    }
}
