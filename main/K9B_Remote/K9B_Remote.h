#ifndef K9B_REMOTE_H__
#define K9B_REMOTE_H__

#define TIME_OUT_SCAN_KP9       10*1000*1000
#define TIME_OUT_PRESS          500*1000
#define RD_GW_ADDR              0x0001
#define MAX_NUM_K9B             5  //hc
#define MAX_NUM_K9ONOFF         5     
#define MAX_NUM_K9B_PRESS_STYLE 12

#include "stdint.h"

typedef struct __attribute__((packed))
{
	uint8_t length;
	uint8_t type_adv;
	uint16_t vid;
	uint8_t frame;
	uint32_t counter;
	uint8_t type_device;
	uint8_t key;
	uint32_t signature;
} switchKP9_proxy_t;

typedef struct
{
	uint8_t Pair_KP9OnOff_Flag;
	uint8_t Button_ID_OnOff;
    int64_t Clock_time_start_pair_onoff;
} Sw_Working_Stt_Str;

struct  button_value_onoff{
	uint32_t last_counter_onoff;
	int64_t time_tongle_onoff; 
};

typedef struct
{
	uint8_t pos_save_onoff_next;
  	uint32_t MacK9B[MAX_NUM_K9ONOFF];
  	uint8_t K9B_BtKey[MAX_NUM_K9ONOFF];
}k9b_para_onoff_t; 

typedef struct
{
	uint8_t Header[4];
	k9b_para_onoff_t K9B_onoff[4];
}flash_K9B_onoff_t;


void K9B_init_task(void);
void K9B_loop_check_pair_time_out(void);

void K9B_set_start_pair_onoff(uint8_t button_index);
void K9B_Pair_OnOff_ClearFlag(void);
void RD_save_flash_K9B_onoff(void);
void K9B_delete_all_onoff_one_btn(uint8_t index);
uint8_t get_flag_pair_onoff(void);

#endif /* K9B_REMOTE_H__ */