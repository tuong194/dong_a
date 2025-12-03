#ifndef _BUTTON_H__
#define _BUTTON_H__

#include "stdbool.h"
#include "stdint.h"
#include "esp_event.h"

#define COUNT_CYCLE_ACTIVE_BTN      5
#define TIME_CHECK_KEEP_MS          1000
#define TIME_COUNT_KEEP             ((TIME_CHECK_KEEP_MS /TICK_INTERVAL) - COUNT_CYCLE_ACTIVE_BTN + 1)
#define TIME_CHECK_LONG_KEEP_MS     5000
#define TIME_COUNT_LONG_KEEP        ((TIME_CHECK_LONG_KEEP_MS /TICK_INTERVAL) - COUNT_CYCLE_ACTIVE_BTN + 1)


#define SET_TIME_OUT_SECOND(x)      (x*1000*1000 - TIME_CHECK_KEEP_MS*1000)

#define TIME_OUT_KICK_OUT           SET_TIME_OUT_SECOND(10)
#define TIME_CYCLE_KICK_OUT         SET_TIME_OUT_SECOND(3)
#define TIME_CYCLE_SET_PAIR         SET_TIME_OUT_SECOND(3)

ESP_EVENT_DECLARE_BASE(BUTTON_EVENT_BASE);

#define BUTTON_CONFIG_WIFI_1 BTN_1
#define BUTTON_CONFIG_WIFI_2 BTN_2

typedef enum 
{
	But_None	= 0x00U,
	But_Press	= 0x01U,
	But_Keeping	= 0x02U,
    But_Long_Keeping = 0x03U
}Button_Stt_Type_t;

typedef enum {
    EVENT_BUTTON_PRESS = 1,
    EVENT_BUTTON_PAIR_K9B,
    EVENT_BUTTON_DELETE_ALL_K9B,
    EVENT_BUTTON_CONFIG_WIFI,
    EVENT_BUTTON_KICK_OUT,
    EVENT_BUTTON_INC_COUNT_KICK_OUT,
} btn_event_id_t;

typedef struct 
{
    uint8_t count_buff_scan;
    uint16_t btn_count_check;
    uint8_t check_keep_flag;
    bool check_press_flag;
    Button_Stt_Type_t button_stt;
}button_param_t;

/**
 * @brief hàm khởi tạo event loop cho button
 * 
 * @param event_handler 
 */
void btn_event_init(esp_event_handler_t event_handler);
void button_manager_init(void);


#endif /* BUTTON_H_ */
