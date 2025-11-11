#ifndef COMMON_H__
#define COMMON_H__

#include "LC8823.h"
#include "Define.h"

#define TIME_DELAY_KICK_OUT        3500*1000
#define TIMEOUT_SAVE_FLASH_CONFIG  3*1000*1000

struct on_off_t{
    uint8_t present;
    uint8_t target;
};

typedef struct {
    uint8_t ele_cnt;
    uint8_t target;
}rd_queue_led_relay;

typedef enum{
    OFF = 0,
    ON,
    TONGLE
}stt_target_coundown;

typedef enum{
    START_OFF = 0,
    START_ON,
    START_RESTORE
}stt_startup_t;

typedef struct{
    int64_t time_start_countdown;
    int64_t time_countdown_s;
    stt_target_coundown stt_target;
}Countdown_t;

typedef struct{
    uint8_t header[4];
    stt_startup_t stt_startup;
    uint8_t on_off[NUM_ELEMENT];
    led_value_onoff_t led_value[NUM_ELEMENT];
}flash_config_value_t;

typedef struct{
    bool is_save_flash;
    int64_t time_start_save_flash;
}save_flash_check_t;

void start_check_save_flash(void);
void control_set_onoff(uint8_t index, uint8_t stt);
void rd_set_countdown(uint8_t index, int64_t time_s, stt_target_coundown target);
void rd_set_status_startup(stt_startup_t stt_startup);
uint8_t get_stt_present(uint8_t index);
void init_control_task(void);

#endif /*  */
