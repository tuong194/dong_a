#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_err.h"
#include "esp_log.h"
#include "Define.h"
#include "relay.h"
#include "util.h"

#define TAG "RELAY"

#define DETECT_ZERO 0
#if DETECT_ZERO
#define NUM_CHECK_DETECH_MAX 150
#define TIME_DETECT_ON 6000  // us
#define TIME_DETECT_OFF 4800 // us
void rd_wait_detect_zero(void);
#endif

relay_t relay[RELAY_NUM] = {
#if RELAY_NUM >= 1
    {
        ELE_1, RELAY1_PIN, RELAY_ACTIVE_HIGH, OFF_STT, false
    }
#endif
#if RELAY_NUM >= 2
    ,{
        ELE_2, RELAY2_PIN, RELAY_ACTIVE_HIGH, OFF_STT, false
    }
#endif
#if RELAY_NUM >= 3
    ,{
        ELE_3, RELAY3_PIN, RELAY_ACTIVE_HIGH, OFF_STT, false
    }
#endif
#if RELAY_NUM >= 4
    ,{
        ELE_4, RELAY4_PIN, RELAY_ACTIVE_HIGH, OFF_STT, false
    }    
#endif
};

static inline uint32_t real_level_set(relay_t *relay, uint8_t state){
    if(relay->active_level == RELAY_ACTIVE_HIGH)
        return state == ON_STT ? 1 : 0;
    else
        return state == ON_STT ? 0 : 1;
}

static inline void relay_set_hw(relay_t *relay){
    printf("RELAY: set relay_%d, %s\n", relay->element + 1, relay->state ? "ON":"OFF");
    gpio_set_level(relay->gpio_pin, real_level_set(relay, relay->state));
    relay->is_control = false;
}

void relay_set_state(relay_t *relay, uint8_t state){
    relay->state = state;
    relay->is_control = true;
}

void relay_control(relay_t *relay){
    if(relay->is_control){
#if DETECT_ZERO
        rd_wait_detect_zero();
        if(relay->state == ON_STT){
            SLEEP_US(TIME_DETECT_ON);
            relay_set_hw(relay);
        }else if(relay->state == OFF_STT){
            SLEEP_US(TIME_DETECT_OFF);
            relay_set_hw(relay);
        }
#else
        relay_set_hw(relay);
#endif
    }
}

void relay_init(relay_t *relay){
    esp_err_t ret = ESP_OK;
    gpio_config_t gpio_config_pin = {
        .pin_bit_mask = 1ULL << (relay->gpio_pin),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ret = gpio_config(&gpio_config_pin);    
    if(ret != ESP_OK){
        ESP_LOGE(TAG, "config gpio %d FAIL", relay->element + 1);
    }
}

#if DETECT_ZERO
static inline uint8_t get_detect_zero_pin(void){
    uint8_t stt = gpio_get_level(DETECT_ZERO_PIN);
    return stt;
}
void rd_wait_detect_zero(void){
    uint8_t over_time_detect = 0;
    uint8_t stt_detect_past1, stt_detect_past2, stt_detect_cur1, stt_detect_cur2;
    stt_detect_past1 = get_detect_zero_pin();
    stt_detect_past2 = get_detect_zero_pin();
    stt_detect_cur1 = get_detect_zero_pin();
    stt_detect_cur2 = get_detect_zero_pin();
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
}
#endif


// TODO

void relay_gpio_config(void){
    #if RELAY_NUM >= 1
        relay_init(&relay[0]);
    #endif
    #if RELAY_NUM >= 2
        relay_init(&relay[1]);
    #endif
    #if RELAY_NUM >= 3    
        relay_init(&relay[2]);
    #endif
    #if RELAY_NUM >= 4
        relay_init(&relay[3]);
    #endif
    #if DETECT_ZERO
        gpio_config_t gpio_config_pin = {
            .pin_bit_mask = 1ULL << DETECT_ZERO_PIN,
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE};
        gpio_config(&gpio_config_pin);
    #endif
}

void set_state_relay(uint8_t index, uint8_t state){
    relay_set_state(&relay[index], state);
}

void relay_handler(void){
    for (size_t i = 0; i < RELAY_NUM; i++)
    {
        relay_control(&relay[i]);
    }
}
