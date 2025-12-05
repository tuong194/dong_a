#include "board.h"
#include "esp_err.h"
#include "Define.h"

static button_handle_t btn_handler[NUM_ELEMENT];
static void board_button_event_cb(void *arg, void *data);

esp_err_t button_gpio_config(void){
    esp_err_t ret = ESP_OK;
    const button_gpio_config_t button_gpio[NUM_ELEMENT] = {
        {
            .gpio_num = BUTTON_PIN1,
            .active_level = TOUCH,
            .disable_pull = false,
        },
        {
            .gpio_num = BUTTON_PIN2,
            .active_level = TOUCH,
            .disable_pull = false,
        },
        {
            .gpio_num = BUTTON_PIN3,
            .active_level = TOUCH,
            .disable_pull = false,
        },
        {
            .gpio_num = BUTTON_PIN4,
            .active_level = TOUCH,
            .disable_pull = false,
        }
    };

    button_config_t btn_config[NUM_ELEMENT] = {
        {
            .press_time = 100,
            .keep_time = 1000,
            .long_keep_time = 5000,
            .button_gpio_config = button_gpio[BTN_1]
        },
        {
            .press_time = 100,
            .keep_time = 1000,
            .long_keep_time = 5000,
            .button_gpio_config = button_gpio[BTN_2]
        },
        {
            .press_time = 100,
            .keep_time = 1000,
            .long_keep_time = 5000,
            .button_gpio_config = button_gpio[BTN_3]
        },
        {
            .press_time = 100,
            .keep_time = 1000,
            .long_keep_time = 5000,
            .button_gpio_config = button_gpio[BTN_4]
        }
    };

    for (size_t i = 0; i < NUM_ELEMENT; i++)
    {
        btn_handler[i] = board_create_button_gpio(&btn_config[i]);
        if(btn_handler[i] == NULL) ret = ESP_FAIL;
    }
    for (size_t i = 0; i < NUM_ELEMENT; i++)
    {
        ret += board_register_callback(btn_handler[i], BUTTON_EVENT_PRESS, board_button_event_cb, NULL);
        ret += board_register_callback(btn_handler[i], BUTTON_EVENT_KEEPING, board_button_event_cb, NULL);
        ret += board_register_callback(btn_handler[i], BUTTON_EVENT_RELEASE_KEEPING, board_button_event_cb, NULL);
        ret += board_register_callback(btn_handler[i], BUTTON_EVENT_LONG_KEEPING, board_button_event_cb, NULL);
        ret += board_register_callback(btn_handler[i], BUTTON_EVENT_RELEASE_LONG_KEEPING, board_button_event_cb, NULL);
    }
    return ret;
}

static void board_button_event_cb(void *arg, void *data){
    button_handle_t btn_handler = (button_handle_t) arg;
    button_event_t event = board_get_event(btn_handler);
    switch (event)
    {
    case BUTTON_EVENT_PRESS:
        printf("[BOARD] btn press\n");
        break;
    case BUTTON_EVENT_KEEPING:
        printf("[BOARD] btn keeping\n");
        break;
    case BUTTON_EVENT_RELEASE_KEEPING:
        printf("[BOARD] btn release keeping\n");
        break;
    case BUTTON_EVENT_LONG_KEEPING:
        printf("[BOARD] btn long keeping\n");
        break;
    case BUTTON_EVENT_RELEASE_LONG_KEEPING:
        printf("[BOARD] btn release long keeping\n");
        break;
    
    default:
        break;
    }
}
