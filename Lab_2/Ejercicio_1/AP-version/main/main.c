#include "ap_wifi.h"
#include "http_server.h"
#include "control_motores.h"

void app_main(void)
{
    motor_init();
    wifi_init_ap();
    start_webserver();
}