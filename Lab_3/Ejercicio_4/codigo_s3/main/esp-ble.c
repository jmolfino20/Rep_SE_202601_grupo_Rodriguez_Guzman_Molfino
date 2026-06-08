#include "nvs_flash.h"
#include "motor.h"
#include "uart_cmd.h"

void app_main(void)
{
    motor_init();
    uart_cmd_init();
}