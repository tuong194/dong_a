#ifndef DEVICE_H__
#define DEVICE_H__

#include <cJSON.h>
#include <stdint.h>


typedef enum
{
    COUNTDOWN = 0,
    ONOFF,
    STATUS_STARTUP,

    ONOFF_BT1,
    ONOFF_BT2,
    ONOFF_BT3,
    ONOFF_BT4,

    RED_ON_1,
    BLUE_ON_1,
    GREEN_ON_1,
    DIM_ON_1,

    RED_OFF_1,
    BLUE_OFF_1,
    GREEN_OFF_1,
    DIM_OFF_1,

    RED_ON_2,
    BLUE_ON_2,
    GREEN_ON_2,
    DIM_ON_2,

    RED_OFF_2,
    BLUE_OFF_2,
    GREEN_OFF_2,
    DIM_OFF_2,

    RED_ON_3,
    BLUE_ON_3,
    GREEN_ON_3,
    DIM_ON_3,

    RED_OFF_3,
    BLUE_OFF_3,
    GREEN_OFF_3,
    DIM_OFF_3,

    RED_ON_4,
    BLUE_ON_4,
    GREEN_ON_4,
    DIM_ON_4,
    
    RED_OFF_4,
    BLUE_OFF_4,
    GREEN_OFF_4,
    DIM_OFF_4,

    UNKNOWN_KEY
}keyID;

typedef enum{
    RED,
    GREEN,
    BLUE
}rgb_t;

typedef struct{
    const char *key;
    keyID ID;    
}key_map_t;

void device_init_rpc_call_back(void);
int rpc_callback_controlDev(cJSON *reqJson, cJSON *respJson);
int rpc_callback_add_home(cJSON *reqJson, cJSON *respJson);
int rpc_callback_del_home(cJSON *reqJson, cJSON *respJson);

int do_controlDev_handle(cJSON *reqJson);
int do_add_home_handle(cJSON *reqJson);
int do_del_home_handle(cJSON *reqJson);

void handle_attributes_message_param_rd(cJSON *attributes_json);

int device_publish_telemetry(const char *key, uint8_t value);
int post_stt_btn_server(uint8_t index, uint8_t stt);
int device_publish_info(void);
int device_rsp_reset_homId(void);

#endif /* DEVICE_H__ */
