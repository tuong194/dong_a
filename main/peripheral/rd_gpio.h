#ifndef RD_GPIO_H_
#define RD_GPIO_H_

#include "stdint.h"
#include "Define.h"

extern gpio_num_t BUTTON_PIN_ARR[NUM_ELEMENT];
extern gpio_num_t RELAY_PIN_ARR[NUM_ELEMENT];

void rd_gpio_init(void);
#endif /*  */
