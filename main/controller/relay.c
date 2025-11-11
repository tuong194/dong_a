#include "relay.h"
#include "rd_gpio.h"
#include "driver/gpio.h"
#include "stdio.h"
#include "util.h"
#include "esp_timer.h"

static int64_t last_time_crl_relay = 0;
static relay_stt_t relay_stt[NUM_ELEMENT] = {0};

void set_stt_relay(uint8_t index, uint8_t stt)
{
    relay_stt[index].stt_relay = stt;
    relay_stt[index].flag_check_control = true;
}

void relay_set_hw(uint8_t index)
{
    if (relay_stt[index].stt_relay)
    {
        gpio_set_level(RELAY_PIN_ARR[index], ON_STT);
    }
    else
    {
        gpio_set_level(RELAY_PIN_ARR[index], OFF_STT);
    }
    relay_stt[index].flag_check_control = false;
}

void control_relay(uint8_t index)
{
    if (relay_stt[index].flag_check_control == true)
    {
        relay_set_hw(index);
        printf("RELAY: set relay_%d\n", index+1);
        #if 0
        if(relay_stt[index].stt_relay){
            if(rd_exceed_us(last_time_crl_relay, TIME_DETECT_ON)){
                relay_set_hw(index);
            }
        }else{
            if(rd_exceed_us(last_time_crl_relay, TIME_DETECT_OFF)){
                relay_set_hw(index);
            }
        }
        #endif
    }
}

uint8_t get_detect_zero_pin(void){
    uint8_t stt = gpio_get_level(DETECT_ZERO_PIN);
    return stt;
}
void rd_wait_detect_zero(void){
    uint8_t over_time_detect = 0;
    uint8_t stt_detect_past1, stt_detect_past2, stt_detect_cur1, stt_detect_cur2;
    stt_detect_past1 = stt_detect_past2 = stt_detect_cur1 = stt_detect_cur2 = get_detect_zero_pin();
    do
    {
        over_time_detect++;
        stt_detect_past1 = get_detect_zero_pin();
        SLEEP_US(50);
        stt_detect_past2 = get_detect_zero_pin();
        SLEEP_US(500);
        stt_detect_cur1 = get_detect_zero_pin();
        SLEEP_US(50);
        stt_detect_cur2 = get_detect_zero_pin();

        if(over_time_detect >= NUM_CHECK_DETECH_MAX){
            printf("detect zero break\n");
            break;
        }
        
    } while (!(stt_detect_past1 != 0 && stt_detect_past2 != 0 && stt_detect_cur1 == 0 && stt_detect_cur2 == 0)); //falling
    last_time_crl_relay = esp_timer_get_time();
}

void relay_manager_loop(void){
    for (size_t i = 0; i < NUM_ELEMENT; i++)
    {
        control_relay(i);
    }   
}