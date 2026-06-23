#include "state_machine.h"
#include "globals.h"
#include "motor.h"
#include "config.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TAG "SM"
#define TICK_MS 20

/* ── Estados ─────────────────────────────────────────────────────── */

typedef enum {
    STATE_IDLE,
    STATE_DETECTED,
    STATE_FAST_SEARCH,
    STATE_ATTACK,
    STATE_BORDER,
} RobotState;

static const char *state_name(RobotState s) {
    switch (s) {
        case STATE_IDLE:        return "IDLE";
        case STATE_DETECTED:    return "DETECTED";
        case STATE_FAST_SEARCH: return "FAST_SEARCH";
        case STATE_ATTACK:      return "ATTACK";
        case STATE_BORDER:      return "BORDER";
        default:                return "?";
    }
}

static inline bool button_pressed(void) {
    return gpio_get_level(BUTTON_PIN) == 0;
}

/* ── Variables de estado ─────────────────────────────────────────── */

static RobotState state     = STATE_IDLE;
static uint32_t   timer_ms  = 0;

/* IDLE: alterna avanzar / girar */
static uint8_t idle_advancing  = 1;
static uint8_t idle_turn_right = 1;

/* FAST_SEARCH: izquierda primero, después derecha */
static uint8_t search_going_left = 1;
static int     search_step       = 0;

/* ATTACK: empuje, después retroceso solo si perdió ID */
static uint8_t atk_pushing = 1;

static void change_state(RobotState new_state) {
    if (new_state != state)
        ESP_LOGI(TAG, "%s -> %s", state_name(state), state_name(new_state));
    state    = new_state;
    timer_ms = 0;

    switch (new_state) {
        case STATE_IDLE:
            idle_advancing = 1;
            break;
        case STATE_FAST_SEARCH:
            search_going_left = 1;
            search_step = 0;
            break;
        case STATE_ATTACK:
            atk_pushing = 1;
            break;
        default:
            break;
    }
}

/* ── Tick de cada estado (no bloqueante, ~20ms) ──────────────────── */

static void tick_idle(void) {
    if (g_id_detected)    { change_state(STATE_DETECTED); return; }
    if (button_pressed()) { change_state(STATE_ATTACK);   return; }

    if (idle_advancing) {
        motor_forward(DUTY_IDLE_FWD, DUTY_IDLE_FWD);
        if (timer_ms >= IDLE_FWD_MS) {
            idle_advancing = 0;
            timer_ms = 0;
        }
    } else {
        if (idle_turn_right)
            motor_right(DUTY_TURN, DUTY_TURN);
        else
            motor_left(DUTY_TURN, DUTY_TURN);

        if (timer_ms >= IDLE_TURN_MS) {
            idle_turn_right = !idle_turn_right;
            idle_advancing = 1;
            timer_ms = 0;
        }
    }
}

static void tick_detected(void) {
    if (button_pressed()) { change_state(STATE_ATTACK);      return; }
    if (!g_id_detected)   { change_state(STATE_FAST_SEARCH); return; }

    motor_forward(DUTY_DETECTED_FWD, DUTY_DETECTED_FWD);
}

static void tick_fast_search(void) {
    if (g_id_detected)    { change_state(STATE_DETECTED); return; }
    if (button_pressed()) { change_state(STATE_ATTACK);   return; }

    if (search_going_left) {
        motor_left(DUTY_FAST_TURN, DUTY_FAST_TURN);
        if (timer_ms >= FAST_SEARCH_TURN_MS) {
            search_step++;
            timer_ms = 0;
            if (search_step >= 3) {
                search_going_left = 0;
                search_step = 0;
            }
        }
    } else {
        motor_right(DUTY_FAST_TURN, DUTY_FAST_TURN);
        if (timer_ms >= FAST_SEARCH_TURN_MS) {
            search_step++;
            timer_ms = 0;
            if (search_step >= FAST_SEARCH_MAX_CYCLES) {
                change_state(STATE_IDLE);
            }
        }
    }
}

static void tick_attack(void) {
    if (atk_pushing) {
        motor_forward(DUTY_ATTACK_FWD, DUTY_ATTACK_FWD);
        if (timer_ms >= ATTACK_FWD_MS) {
            atk_pushing = 0;
            timer_ms = 0;
        }
    } else {
        motor_backward(DUTY_ATTACK_BACK, DUTY_ATTACK_BACK);
        if (timer_ms >= ATTACK_BACK_MS) {
            change_state(g_id_detected ? STATE_DETECTED : STATE_IDLE);
        }
    }
}

/* ── Task principal (Core 0) — un tick cada 20ms ─────────────────── */

void state_machine_task(void *arg) {
    ESP_LOGI(TAG, "Iniciando en estado IDLE (tick=%d ms)", TICK_MS);
    change_state(STATE_IDLE);

    while (1) {
        /* ── Prioridad 1: Audio override (siempre obedece) ── */
        if (g_audio_override) {
            vTaskDelay(pdMS_TO_TICKS(TICK_MS));
            continue;
        }

        /* ── Prioridad 2: Borde (siempre, incluso durante ATTACK) ── */
        if (g_border_detected) {
            if (state != STATE_BORDER) {
                ESP_LOGW(TAG, "BORDE detectado (desde %s)", state_name(state));
                state = STATE_BORDER;
                timer_ms = 0;
            }
            motor_right(DUTY_BORDER_TURN, DUTY_BORDER_TURN);
            timer_ms += TICK_MS;
            vTaskDelay(pdMS_TO_TICKS(TICK_MS));
            continue;
        }
        if (state == STATE_BORDER) {
            ESP_LOGW(TAG, "BORDE despejado -> IDLE");
            change_state(STATE_IDLE);
        }

        /* ── Máquina de estados ── */
        switch (state) {
            case STATE_IDLE:        tick_idle();        break;
            case STATE_DETECTED:    tick_detected();    break;
            case STATE_FAST_SEARCH: tick_fast_search(); break;
            case STATE_ATTACK:      tick_attack();      break;
            default:                change_state(STATE_IDLE); break;
        }

        timer_ms += TICK_MS;
        vTaskDelay(pdMS_TO_TICKS(TICK_MS));
    }
}
