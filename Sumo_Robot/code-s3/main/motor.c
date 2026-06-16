#include "motor.h"
#include "config.h"
#include "driver/gpio.h"
#include "driver/ledc.h"

void motor_init(void) {
    gpio_set_direction(MOTOR_IN1, GPIO_MODE_OUTPUT);
    gpio_set_direction(MOTOR_IN2, GPIO_MODE_OUTPUT);
    gpio_set_direction(MOTOR_IN3, GPIO_MODE_OUTPUT);
    gpio_set_direction(MOTOR_IN4, GPIO_MODE_OUTPUT);

    ledc_timer_config_t timer = {
        .speed_mode      = LEDC_LOW_SPEED_MODE,
        .timer_num       = LEDC_TIMER_0,
        .duty_resolution = MOTOR_PWM_RES,
        .freq_hz         = MOTOR_PWM_FREQ,
        .clk_cfg         = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&timer);

    ledc_channel_config_t chA = {
        .gpio_num   = MOTOR_ENA,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel    = LEDC_CHANNEL_0,
        .timer_sel  = LEDC_TIMER_0,
        .duty       = 0,
    };
    ledc_channel_config_t chB = {
        .gpio_num   = MOTOR_ENB,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel    = LEDC_CHANNEL_1,
        .timer_sel  = LEDC_TIMER_0,
        .duty       = 0,
    };
    ledc_channel_config(&chA);
    ledc_channel_config(&chB);
}

static void set_pwm(uint8_t dutyA, uint8_t dutyB) {
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, dutyA);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, dutyB);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
}

void motor_stop(void) {
    set_pwm(0, 0);
}

void motor_forward(uint8_t dutyA, uint8_t dutyB) {
    gpio_set_level(MOTOR_IN1, 0); gpio_set_level(MOTOR_IN2, 1);
    gpio_set_level(MOTOR_IN3, 1); gpio_set_level(MOTOR_IN4, 0);
    set_pwm(dutyA, dutyB);
}

void motor_backward(uint8_t dutyA, uint8_t dutyB) {
    gpio_set_level(MOTOR_IN1, 1); gpio_set_level(MOTOR_IN2, 0);
    gpio_set_level(MOTOR_IN3, 0); gpio_set_level(MOTOR_IN4, 1);
    set_pwm(dutyA, dutyB);
}

void motor_right(uint8_t dutyA, uint8_t dutyB) {
    /* Motor A forward, Motor B backward → gira a la derecha */
    gpio_set_level(MOTOR_IN1, 0); gpio_set_level(MOTOR_IN2, 1);
    gpio_set_level(MOTOR_IN3, 0); gpio_set_level(MOTOR_IN4, 1);
    set_pwm(dutyA, dutyB);
}

void motor_left(uint8_t dutyA, uint8_t dutyB) {
    /* Motor A backward, Motor B forward → gira a la izquierda */
    gpio_set_level(MOTOR_IN1, 1); gpio_set_level(MOTOR_IN2, 0);
    gpio_set_level(MOTOR_IN3, 1); gpio_set_level(MOTOR_IN4, 0);
    set_pwm(dutyA, dutyB);
}
