#include "motor.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "config.h"

#define IN1 5
#define IN2 6
#define IN3 8
#define IN4 9

#define ENA 4
#define ENB 7

void motor_init() {
    // GPIO dirección
    gpio_set_direction(IN1, GPIO_MODE_OUTPUT);
    gpio_set_direction(IN2, GPIO_MODE_OUTPUT);
    gpio_set_direction(IN3, GPIO_MODE_OUTPUT);
    gpio_set_direction(IN4, GPIO_MODE_OUTPUT);

    // PWM config
    ledc_timer_config_t timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 1000,
        .duty_resolution = LEDC_TIMER_8_BIT
    };
    ledc_timer_config(&timer);

    ledc_channel_config_t ch0 = {
        .channel = LEDC_CHANNEL_0,
        .duty = 180,
        .gpio_num = ENA,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_sel = LEDC_TIMER_0
    };

    ledc_channel_config_t ch1 = {
        .channel = LEDC_CHANNEL_1,
        .duty = 180,
        .gpio_num = ENB,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_sel = LEDC_TIMER_0
    };

    ledc_channel_config(&ch0);
    ledc_channel_config(&ch1);
}

void move_forward() {
    gpio_set_level(IN1, 1); gpio_set_level(IN2, 0);
    gpio_set_level(IN3, 1); gpio_set_level(IN4, 0);
}

void move_backward() {
    gpio_set_level(IN1, 0); gpio_set_level(IN2, 1);
    gpio_set_level(IN3, 0); gpio_set_level(IN4, 1);
}

void move_left() {
    gpio_set_level(IN1, 0); gpio_set_level(IN2, 1);
    gpio_set_level(IN3, 1); gpio_set_level(IN4, 0);
}

void move_right() {
    gpio_set_level(IN1, 1); gpio_set_level(IN2, 0);
    gpio_set_level(IN3, 0); gpio_set_level(IN4, 1);
}

void stop() {
    gpio_set_level(IN1, 0); gpio_set_level(IN2, 0);
    gpio_set_level(IN3, 0); gpio_set_level(IN4, 0);
}
