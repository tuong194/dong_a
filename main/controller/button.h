#ifndef _BUTTON_H__
#define _BUTTON_H__

#include "stdbool.h"
#include "stdint.h"
#include "esp_event.h"

#define COUNT_CYCLE_ACTIVE_BTN      5
#define TIME_OUT_KICK_OUT           10*1000*1000
#define TIME_CYCLE_KICK_OUT         3*1000*1000 
#define TIME_CYCLE_SET_PAIR         3*1000*1000 
#define TIME_CYCLE_CONFIG_WIFI      2*1000*1000 

#define TIME_CHECK_HOLD_MS          1000
#define TIME_COUNT_HOLD             100//TIME_CHECK_HOLD_MS/(COUNT_CYCLE_ACTIVE_BTN*10)  //10ms scan 1 lan
 
ESP_EVENT_DECLARE_BASE(BUTTON_EVENT_BASE);

typedef enum 
{
	But_None	= 0x00U,
	But_Press	= 0x01U,
	But_Keeping	= 0x02U
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
    uint8_t btn_count_check; //check press, hold
    bool check_keep_flag;
    bool check_press_flag;
    Button_Stt_Type_t button_stt;
}button_param_t;


void btn_event_init(esp_event_handler_t event_handler);
void btn_manager_loop(void);

#endif /* BUTTON_H_ */
