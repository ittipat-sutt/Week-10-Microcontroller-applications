// main.c - ตัวอย่างขั้นสูง: 4 Buttons บน ADC Pin เดียว
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "esp_log.h"

// กำหนด pins
#define ADC_BUTTONS_PIN   ADC1_CHANNEL_4    // GPIO32
#define LED1_PIN          GPIO_NUM_19       // LED1 - UP
#define LED2_PIN          GPIO_NUM_21       // LED2 - DOWN  
#define LED3_PIN          GPIO_NUM_22       // LED3 - LEFT
#define LED4_PIN          GPIO_NUM_23       // LED4 - RIGHT

#define ADC_SAMPLES       32

static const char *TAG = "4BUTTON_ADC";
static esp_adc_cal_characteristics_t *adc_chars;

// Button definitions
typedef enum {
    BTN_NONE = 0,
    BTN_UP,
    BTN_DOWN, 
    BTN_LEFT,
    BTN_RIGHT
} button_id_t;

// Voltage thresholds (ปรับตามค่าจริง)
typedef struct {
    button_id_t id;
    uint32_t min_voltage;
    uint32_t max_voltage;
    const char* name;
    gpio_num_t led_pin;
} button_threshold_t;

static const button_threshold_t button_thresholds[] = {
    {BTN_NONE,  0,    400,  "NONE",  GPIO_NUM_NC},
    {BTN_UP,    400,  800,  "UP",    LED1_PIN},
    {BTN_DOWN,  800,  1200, "DOWN",  LED2_PIN},
    {BTN_LEFT,  1200, 1600, "LEFT",  LED3_PIN},
    {BTN_RIGHT, 1600, 3300, "RIGHT", LED4_PIN}
};

#define NUM_BUTTONS (sizeof(button_thresholds) / sizeof(button_threshold_t))

void init_adc(void)
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC_BUTTONS_PIN, ADC_ATTEN_DB_11);
    
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 
                           1100, adc_chars);
    
    ESP_LOGI(TAG, "ADC initialized and calibrated");
}

uint32_t read_voltage_mv(void)
{
    uint32_t adc_reading = 0;
    
    for (int i = 0; i < ADC_SAMPLES; i++) {
        adc_reading += adc1_get_raw(ADC_BUTTONS_PIN);
    }
    adc_reading /= ADC_SAMPLES;
    
    return esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
}

button_id_t get_pressed_button(uint32_t voltage_mv)
{
    for (int i = 0; i < NUM_BUTTONS; i++) {
        if (voltage_mv >= button_thresholds[i].min_voltage && 
            voltage_mv < button_thresholds[i].max_voltage) {
            return button_thresholds[i].id;
        }
    }
    return BTN_NONE;
}

void control_leds(button_id_t pressed_button)
{
    // ปิด LEDs ทั้งหมด
    gpio_set_level(LED1_PIN, 0);
    gpio_set_level(LED2_PIN, 0);
    gpio_set_level(LED3_PIN, 0);
    gpio_set_level(LED4_PIN, 0);
    
    // เปิด LED ที่ตรงกับ button ที่กด
    for (int i = 0; i < NUM_BUTTONS; i++) {
        if (button_thresholds[i].id == pressed_button && 
            button_thresholds[i].led_pin != GPIO_NUM_NC) {
            gpio_set_level(button_thresholds[i].led_pin, 1);
            break;
        }
    }
}

void button_monitor_task(void *pvParameters)
{
    button_id_t current_button = BTN_NONE;
    button_id_t last_button = BTN_NONE;
    
    ESP_LOGI(TAG, "=== 4-Button ADC Monitor ===");
    ESP_LOGI(TAG, "UP: LED1, DOWN: LED2, LEFT: LED3, RIGHT: LED4");
    
    while(1) {
        uint32_t voltage = read_voltage_mv();
        current_button = get_pressed_button(voltage);
        
        if (current_button != last_button) {
            ESP_LOGI(TAG, "Voltage: %dmV", voltage);
            
            // หา button name
            const char* button_name = "UNKNOWN";
            for (int i = 0; i < NUM_BUTTONS; i++) {
                if (button_thresholds[i].id == current_button) {
                    button_name = button_thresholds[i].name;
                    break;
                }
            }
            
            ESP_LOGI(TAG, "Button: %s", button_name);
            control_leds(current_button);
            last_button = current_button;
        }
        
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void app_main(void)
{
    // เริ่มต้น ADC
    init_adc();
    
    // กำหนดค่า GPIO สำหรับ LEDs
    gpio_config_t led_config = {
        .pin_bit_mask = (1ULL << LED1_PIN) | (1ULL << LED2_PIN) |
                        (1ULL << LED3_PIN) | (1ULL << LED4_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&led_config);
    
    // ปิด LEDs เริ่มต้น
    control_leds(BTN_NONE);
    
    ESP_LOGI(TAG, "System initialized");
    ESP_LOGI(TAG, "ADC: GPIO32, LEDs: GPIO19-23");
    
    // แสดง voltage thresholds
    ESP_LOGI(TAG, "Voltage Thresholds:");
    for (int i = 0; i < NUM_BUTTONS; i++) {
        ESP_LOGI(TAG, "%s: %d-%dmV", 
                button_thresholds[i].name,
                button_thresholds[i].min_voltage,
                button_thresholds[i].max_voltage);
    }
    
    xTaskCreate(button_monitor_task, "button_monitor", 4096, NULL, 10, NULL);
}
