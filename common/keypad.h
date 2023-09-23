#ifndef KEYPAD_H
#define KEYPAD_H

#include <driver/gpio.h>

#define KEYPAD_DEBOUNCING 100
#define KEYPAD_STACKSIZE  5

esp_err_t keypad_initalize(gpio_num_t keypad_pins[8]);

char keypad_getkey();

void keypad_delete(void);

#endif