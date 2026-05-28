#include <stdio.h>
#include <math.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_timer.h"
#include "inference.h"

void app_main(void)
{
    for (float x = 0; x < 6.28f; x += 0.1f)
    {
        float y;

        int64_t start = esp_timer_get_time();
        inference(x, &y);
        int64_t end = esp_timer_get_time();

        printf("x=%f y=%f time=%lld us\n",
               x, y, (long long)(end - start));
        
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
