#include "keypad.h"

#include <memory.h>
#include <time.h>
#include <esp_log.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

const char keypad[] = { 
    '1', '2', '3', 'A',
    '4', '5', '6', 'B',
    '7', '8', '9', 'C',
    '*', '0', '#', 'D'
};  

static gpio_num_t _keypad_pins[8];

time_t time_old_isr = 0;

QueueHandle_t keypad_queue;


void intr_click_handler(void *args);


void turnon_rows()
{
    for(int i = 4; i < 8; i++) /// Columns
    {
        gpio_set_pull_mode(_keypad_pins[i], GPIO_PULLDOWN_ONLY);
    }
    for(int i = 0; i < 4; i++) /// Rows
    {
        gpio_set_pull_mode(_keypad_pins[i], GPIO_PULLUP_ONLY);
        gpio_intr_enable(_keypad_pins[i]);
    }
}

void turnon_cols()
{
    for(int i = 0; i < 4; i++) /// Rows
    {
        gpio_intr_disable(_keypad_pins[i]);
        gpio_set_pull_mode(_keypad_pins[i], GPIO_PULLDOWN_ONLY);
    }
    for(int i = 4; i < 8; i++) /// Columns
    {
        gpio_set_pull_mode(_keypad_pins[i], GPIO_PULLUP_ONLY);
    }
}

esp_err_t keypad_initalize(gpio_num_t keypad_pins[8])
{
    memcpy(_keypad_pins, keypad_pins, 8*sizeof(gpio_num_t));


    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_install_isr_service(ESP_INTR_FLAG_EDGE));
    for(int i = 0; i < 4; i++) /// Rows
    {
        gpio_intr_disable(keypad_pins[i]);
        gpio_set_direction(keypad_pins[i], GPIO_MODE_INPUT);
        gpio_set_intr_type(keypad_pins[i], GPIO_INTR_NEGEDGE);
        ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_isr_handler_add(_keypad_pins[i], (void*)intr_click_handler, (void*)i));
        
    }
    for(int i = 4; i < 8; i++)
    {
        gpio_set_direction(keypad_pins[i], GPIO_MODE_INPUT);
    }

    keypad_queue = xQueueCreate(5, sizeof(char));
    if(keypad_queue == NULL)
        return ESP_ERR_NO_MEM;

    turnon_rows();

    return ESP_OK;
}

void intr_click_handler(void* args)
{
    int index = (int)(args);
    
    time_t time_now_isr = time(NULL);
    time_t time_isr = (time_now_isr - time_old_isr)*1000L;
    
    if(time_isr >= KEYPAD_DEBOUNCING)
    {
        turnon_cols();
        for(int j = 4; j < 8; j++)
        {
            if(!gpio_get_level(_keypad_pins[j]))
            {
                xQueueSendFromISR(keypad_queue, &keypad[index*4 + j - 4], NULL);
                break;
            }
        }
        turnon_rows();
    }
    time_old_isr = time_now_isr;
    
}

char keypad_getkey()
{
    char key;
    if(!uxQueueMessagesWaiting(keypad_queue)) 
        return '\0';
    xQueueReceive(keypad_queue, &key, portMAX_DELAY);    
    return key;
}

void keypad_delete()
{
    for(int i = 0; i < 8; i++)
    {   
        gpio_isr_handler_remove(_keypad_pins[i]);
        gpio_set_direction(_keypad_pins[i], GPIO_MODE_DISABLE);
    }
    vQueueDelete(keypad_queue);
}