#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

void app_main(void)
{
    int SW1_status = 0;
    int LED1_status = 0;
    gpio_set_direction(16, GPIO_MODE_OUTPUT);
    gpio_set_direction(15, GPIO_MODE_INPUT);

    int SW2_status = 0;
    int LED2_status = 0;
    gpio_set_direction(18, GPIO_MODE_OUTPUT);
    gpio_set_direction(17, GPIO_MODE_INPUT);
    while(1)
    {
        SW1_status = gpio_get_level(15);
        LED1_status = !SW1_status;
        gpio_set_level(16, LED1_status);

        SW2_status = gpio_get_level(17);
        LED2_status = !SW2_status;
        gpio_set_level(18, LED2_status);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
