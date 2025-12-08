#ifndef _RD_RELAY_H__
#define _RD_RELAY_H__

#include "soc/gpio_num.h"
#include "stdint.h"
#include "stdbool.h"

typedef enum {
    RELAY_ACTIVE_HIGH = 1,
    RELAY_ACTIVE_LOW  = 0
} relay_active_t;

typedef struct {
    uint8_t element;
    gpio_num_t gpio_pin;
    relay_active_t active_level;
    uint8_t state;
    bool is_control;
}relay_t;

void set_state_relay(uint8_t index, uint8_t state);
void relay_gpio_config(void);
void relay_handler(void);

#endif /*  */
