#include "state_machine.h"
#include "globals.h"
#include "motor.h"
#include "config.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TAG "SM"

typedef enum {
    STATE_IDLE,
    STATE_DETECTED,
    STATE_FAST_SEARCH,
    STATE_ATTACK,
} RobotState;

static const char *state_name(RobotState s) {
    switch (s) {
        case STATE_IDLE:        return "IDLE";
        case STATE_DETECTED:    return "DETECTED";
        case STATE_FAST_SEARCH: return "FAST_SEARCH";
        case STATE_ATTACK:      return "ATTACK";
        default:                return "?";
    }
}

static inline bool button_pressed(void) {
    return gpio_get_level(BUTTON_PIN) == 0;   /* active-low */
}

/* Espera ms milisegundos. Retorna false si borde interrumpe. */
static bool delay_ms(uint32_t ms) {
    for (uint32_t elapsed = 0; elapsed < ms; elapsed += 10) {
        if (g_border_detected) return false;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    return true;
}

/* ── Estados ─────────────────────────────────────────────────────── */

static RobotState run_idle(void) {
    /* Avanza chequeando ID cada 50ms en vez de esperar 4s ciego */
    motor_forward(DUTY_IDLE_FWD, DUTY_IDLE_FWD);
    for (int i = 0; i < IDLE_FWD_MS / 50; i++) {
        if (!delay_ms(50)) return STATE_IDLE;
        if (g_id_detected) {
            ESP_LOGI(TAG, "IDLE → DETECTED  [causa: ID detectado]");
            return STATE_DETECTED;
        }
        if (button_pressed()) {
            ESP_LOGI(TAG, "IDLE → ATTACK  [causa: botón]");
            return STATE_ATTACK;
        }
    }

    /* Gira chequeando ID cada 50ms */
    motor_right(DUTY_TURN, DUTY_TURN);
    for (int i = 0; i < IDLE_TURN_MS / 50; i++) {
        if (!delay_ms(50)) return STATE_IDLE;
        if (g_id_detected) {
            ESP_LOGI(TAG, "IDLE → DETECTED  [causa: ID detectado durante giro]");
            return STATE_DETECTED;
        }
        if (button_pressed()) {
            ESP_LOGI(TAG, "IDLE → ATTACK  [causa: botón durante giro]");
            return STATE_ATTACK;
        }
    }

    return STATE_IDLE;
}

static RobotState run_detected(void) {
    motor_forward(DUTY_DETECTED_FWD, DUTY_DETECTED_FWD);
    if (!delay_ms(50)) {
        ESP_LOGW(TAG, "DETECTED: avance interrumpido por borde -> forzando IDLE");
        return STATE_IDLE;
    }

    if (button_pressed()) {
        ESP_LOGI(TAG, "DETECTED → ATTACK  [causa: botón presionado]");
        return STATE_ATTACK;
    }
    if (!g_id_detected) {
        ESP_LOGI(TAG, "DETECTED → FAST_SEARCH  [causa: ID perdido]");
        return STATE_FAST_SEARCH;
    }

    return STATE_DETECTED;
}

static RobotState run_fast_search(void) {
    /* Busca girando a la izquierda */
    for (int i = 0; i < 3; i++) {
        ESP_LOGI(TAG, "FAST_SEARCH: giro izquierda %d/3", i + 1);
        motor_left(DUTY_FAST_TURN, DUTY_FAST_TURN);
        if (!delay_ms(FAST_SEARCH_TURN_MS)) {
            ESP_LOGW(TAG, "FAST_SEARCH: giro izq %d interrumpido por borde -> forzando IDLE", i + 1);
            return STATE_IDLE;
        }
        if (g_id_detected) {
            ESP_LOGI(TAG, "FAST_SEARCH → DETECTED  [causa: ID encontrado girando izq]");
            return STATE_DETECTED;
        }
        if (button_pressed()) {
            ESP_LOGI(TAG, "FAST_SEARCH → ATTACK  [causa: botón]");
            return STATE_ATTACK;
        }
    }

    /* Barre a la derecha */
    for (int i = 0; i < FAST_SEARCH_MAX_CYCLES; i++) {
        ESP_LOGI(TAG, "FAST_SEARCH: giro derecha %d/%d", i + 1, FAST_SEARCH_MAX_CYCLES);
        motor_right(DUTY_FAST_TURN, DUTY_FAST_TURN);
        if (!delay_ms(FAST_SEARCH_TURN_MS)) {
            ESP_LOGW(TAG, "FAST_SEARCH: giro der %d interrumpido por borde -> forzando IDLE", i + 1);
            return STATE_IDLE;
        }
        if (g_id_detected) {
            ESP_LOGI(TAG, "FAST_SEARCH → DETECTED  [causa: ID encontrado girando der]");
            return STATE_DETECTED;
        }
        if (button_pressed()) {
            ESP_LOGI(TAG, "FAST_SEARCH → ATTACK  [causa: botón]");
            return STATE_ATTACK;
        }
    }

    ESP_LOGI(TAG, "FAST_SEARCH → IDLE  [causa: búsqueda agotada]");
    return STATE_IDLE;
}

static RobotState run_attack(void) {
    ESP_LOGI(TAG, "ATTACK: empuje rápido (%d ms, duty=%d)", ATTACK_FWD_MS, DUTY_ATTACK_FWD);
    motor_forward(DUTY_ATTACK_FWD, DUTY_ATTACK_FWD);
    vTaskDelay(pdMS_TO_TICKS(ATTACK_FWD_MS));

    ESP_LOGI(TAG, "ATTACK: retroceso lento (%d ms, duty=%d)", ATTACK_BACK_MS, DUTY_ATTACK_BACK);
    motor_backward(DUTY_ATTACK_BACK, DUTY_ATTACK_BACK);
    vTaskDelay(pdMS_TO_TICKS(ATTACK_BACK_MS));

    motor_stop();
    vTaskDelay(pdMS_TO_TICKS(100));

    RobotState next = g_id_detected ? STATE_DETECTED : STATE_IDLE;
    ESP_LOGI(TAG, "ATTACK → %s  [causa: %s]",
             state_name(next),
             g_id_detected ? "ID aún visible" : "sin ID");
    return next;
}

/* ── Task principal (Core 0) ─────────────────────────────────────── */

void state_machine_task(void *arg) {
    RobotState state      = STATE_IDLE;
    RobotState prev_state = STATE_IDLE;
    uint8_t    in_border  = 0;
    uint8_t    in_audio   = 0;

    ESP_LOGI(TAG, "Iniciando en estado IDLE");

    while (1) {
        /* ── Interrupción borde ───────────────────────────────── */
        if (g_border_detected && !in_border) {
            in_border = 1;
            ESP_LOGW(TAG, "INT BORDE: borde detectado → girando derecha (duty=%d)", DUTY_BORDER_TURN);
        }
        while (g_border_detected) {
            if (!g_audio_override)
                motor_right(DUTY_BORDER_TURN, DUTY_BORDER_TURN);
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        if (in_border) {
            in_border = 0;
            state = STATE_IDLE;
            ESP_LOGW(TAG, "INT BORDE: borde despejado → forzando IDLE");
        }

        /* ── Interrupción audio ───────────────────────────────── */
        if (g_audio_override && !in_audio) {
            in_audio = 1;
            ESP_LOGW(TAG, "INT AUDIO: override activo → cediendo motor");
        }
        if (!g_audio_override && in_audio) {
            in_audio = 0;
            ESP_LOGW(TAG, "INT AUDIO: override liberado → retomando estado %s", state_name(state));
        }
        if (g_audio_override) {
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }

        /* ── Máquina de estados ───────────────────────────────── */
        prev_state = state;
        switch (state) {
            case STATE_IDLE:        state = run_idle();        break;
            case STATE_DETECTED:    state = run_detected();    break;
            case STATE_FAST_SEARCH: state = run_fast_search(); break;
            case STATE_ATTACK:      state = run_attack();      break;
            default:                state = STATE_IDLE;        break;
        }

    }
}
