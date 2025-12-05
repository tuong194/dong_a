#ifndef RD_BOARD_H__
#define RD_BOARD_H__

#include "soc/gpio_num.h"
#include "stdint.h"
#include "stdbool.h"
#include "esp_err.h"

typedef void *button_handle_t;
typedef void (* button_cb_t)(void *button_handle, void *usr_data);

typedef struct {
    gpio_num_t gpio_num;              /**< num of gpio */
    uint8_t active_level;          /**< gpio level when press down */
    bool disable_pull;            /**< disable internal pull or not */
} button_gpio_config_t;

typedef struct{
    uint16_t press_time;
    uint16_t keep_time;
    uint16_t long_keep_time;
    button_gpio_config_t button_gpio_config;
}button_config_t;

typedef enum{
    BUTTON_EVENT_PRESS,
    BUTTON_EVENT_KEEPING,
    BUTTON_EVENT_RELEASE_KEEPING,
    BUTTON_EVENT_LONG_KEEPING,
    BUTTON_EVENT_RELEASE_LONG_KEEPING,
    BUTTON_EVENT_MAX,
    BUTTON_EVENT_NONE_PRESS    
}button_event_t;

typedef enum 
{
	But_None	= 0x00U,
	But_Press	= 0x01U,
	But_Keeping	= 0x02U,
    But_Long_Keeping = 0x03U
}button_stt_t;

button_handle_t board_create_button_gpio(const button_config_t *config);
esp_err_t board_register_callback(button_handle_t btn_handle, button_event_t event, button_cb_t cb, void *usr_data);
button_event_t board_get_event(button_handle_t btn_handle);

#endif /*  */
