#include "control_motores.h"
#include "config.h"

#include "driver/gpio.h"
#include "driver/ledc.h"

void motor_init() {

    gpio_set_direction(IN1, GPIO_MODE_OUTPUT);
    gpio_set_direction(IN2, GPIO_MODE_OUTPUT);
    gpio_set_direction(IN3, GPIO_MODE_OUTPUT);
    gpio_set_direction(IN4, GPIO_MODE_OUTPUT);

    gpio_set_direction(ENA, GPIO_MODE_OUTPUT);
    gpio_set_direction(ENB, GPIO_MODE_OUTPUT);

    gpio_set_level(ENA, 1);
    
    gpio_set_level(ENB, 1);
    
    

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
    gpio_set_level(IN1, 1); gpio_set_level(IN2, 0);
    gpio_set_level(IN3, 0); gpio_set_level(IN4, 1);
}

void move_right() {
    gpio_set_level(IN1, 0); gpio_set_level(IN2, 1);
    gpio_set_level(IN3, 1); gpio_set_level(IN4, 0);
}

void stop() {
    gpio_set_level(IN1, 0); gpio_set_level(IN2, 0);
    gpio_set_level(IN3, 0); gpio_set_level(IN4, 0);
}