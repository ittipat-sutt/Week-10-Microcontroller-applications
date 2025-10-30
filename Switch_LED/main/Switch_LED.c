#include <stdio.h>
#include <driver/gpio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void)
{
    int SW1_Status = 0;
    int LED1_Status = 0;
    int SW2_Status = 0;
    int LED2_Status = 0;

    gpio_set_direction(16, GPIO_MODE_OUTPUT);
    gpio_set_direction(18, GPIO_MODE_OUTPUT);

    gpio_set_direction(15, GPIO_MODE_INPUT);
    gpio_set_pull_mode(17, GPIO_MODE_INPUT);

    while (1)
    {
        SW1_Status = gpio_get_level(15);
        SW2_Status = gpio_get_level(17);

        LED1_Status = !SW1_Status;
        LED2_Status = !SW2_Status;

        gpio_set_level(16, LED1_Status);
        gpio_set_level(18, LED2_Status);

        vTaskDelay(pdMS_TO_TICKS(100));
    }
    


}
