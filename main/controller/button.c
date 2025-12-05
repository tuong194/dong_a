#include <stdio.h>
#include "esp_timer.h"
#include "esp_log.h"
#include "driver/gpio.h"

#include "button.h"
#include "util.h"
#include "rd_gpio.h"


ESP_EVENT_DEFINE_BASE(BUTTON_EVENT_BASE);
static esp_event_loop_handle_t btn_event_loop;
static button_param_t button_para[NUM_ELEMENT] = {0};
static int64_t time_start_hold = 0;
static uint8_t index_btn = 0;

extern uint8_t get_flag_pair_onoff(void);


static inline void clear_flag_press_one_btn(uint8_t index)
{
    button_para[index].check_press_flag = false;
}

static bool check_press_one_btn(uint8_t index)
{
    if (button_para[index].check_press_flag)
    {
        clear_flag_press_one_btn(index);
        return true;
    }
    return false;
}
static inline uint8_t btn_is_keeping(uint8_t index)
{
    return button_para[index].check_keep_flag;
}
static inline void clear_check_keep(uint8_t index)
{
    button_para[index].check_keep_flag = 0;
}

static inline uint8_t get_num_btn_keeping(){
    uint8_t number = 0;
    #ifdef BTN_1
        number += btn_is_keeping(BTN_1);
    #endif
    #ifdef BTN_2
        number += btn_is_keeping(BTN_2);
    #endif
    #ifdef BTN_3
        number += btn_is_keeping(BTN_3);
    #endif
    #ifdef BTN_4
        number += btn_is_keeping(BTN_4);
    #endif
    return number;
}

static void scan_one_btn(uint8_t index)
{
    if (gpio_get_level(BUTTON_PIN_ARR[index]) == TOUCH)
    {
        button_para[index].count_buff_scan++;

        if (button_para[index].count_buff_scan == COUNT_CYCLE_ACTIVE_BTN && button_para[index].btn_count_check == 0)
        {
            button_para[index].btn_count_check = 1;
        }
        else if ((button_para[index].count_buff_scan > COUNT_CYCLE_ACTIVE_BTN) || (button_para[index].btn_count_check > 1))
        {
            button_para[index].btn_count_check += 1;
        }
    }
    else
    {
        button_para[index].btn_count_check = 0;
        button_para[index].count_buff_scan = 0;
    }
    if ((button_para[index].btn_count_check == 1) && (button_para[index].button_stt == But_None))
    {
        button_para[index].button_stt = But_Press;
    }
    else if (button_para[index].btn_count_check >= TIME_COUNT_KEEP && button_para[index].button_stt == But_Press)
    {
        printf("button %u is keeping\n", index + 1);
        button_para[index].button_stt = But_Keeping;
        button_para[index].check_keep_flag = 1;
        get_tick_time(&time_start_hold);
    }
    else if (button_para[index].btn_count_check >= TIME_COUNT_LONG_KEEP && button_para[index].button_stt == But_Keeping)
    {
        printf("button %u is long keeping\n", index + 1);
        button_para[index].check_keep_flag = 0;
        button_para[index].button_stt = But_Long_Keeping;      
    }

    else if (button_para[index].btn_count_check == 0)
    {
        if(button_para[index].button_stt == But_Long_Keeping){
            printf("release long keeping btn %u\n", index + 1);

        }
        else if (button_para[index].button_stt == But_Keeping)
        {
            button_para[index].check_keep_flag = 0;
            get_tick_time(&time_start_hold);
        }
        else if (button_para[index].button_stt == But_Press)
        {
            printf("button %u press\n", index + 1);
            button_para[index].check_press_flag = true;
        }
        button_para[index].button_stt = But_None;
        clear_check_keep(index);
    }
}

void scan_all_btn(void)
{
    for (size_t i = 0; i < NUM_ELEMENT; i++)
    {
        scan_one_btn(i);
    }
}

int btn_check_press(void)
{
    static uint8_t i = 0;
    for (i = 0; i < NUM_ELEMENT; i++)
    {
        if (check_press_one_btn(i))
        {
            esp_event_post_to(btn_event_loop, BUTTON_EVENT_BASE, EVENT_BUTTON_PRESS, &i, 1, portMAX_DELAY);
            if (index_btn == i && get_flag_pair_onoff())
            {
                esp_event_post_to(btn_event_loop, BUTTON_EVENT_BASE, EVENT_BUTTON_DELETE_ALL_K9B, &i, 1, portMAX_DELAY);
                index_btn = 0xff;
            }
            else if (!get_flag_pair_onoff())
                index_btn = 0xff;
            return i;
        }
    }
    return -1;
}

int btn_check_keeping(){
    static uint8_t count_kick_out = 0;
#if NUM_ELEMENT == 1
    if(button_para[BUTTON_CONFIG_WIFI_1].button_stt == But_Long_Keeping){
        esp_event_post_to(btn_event_loop, BUTTON_EVENT_BASE, EVENT_BUTTON_CONFIG_WIFI, NULL, 0, portMAX_DELAY);
        button_para[BUTTON_CONFIG_WIFI_1].button_stt = But_None;
        return 1;
    }
#else
    if(button_para[BUTTON_CONFIG_WIFI_1].button_stt == But_Long_Keeping && button_para[BUTTON_CONFIG_WIFI_2].button_stt == But_Long_Keeping){
        button_para[BUTTON_CONFIG_WIFI_1].button_stt = But_None;
        button_para[BUTTON_CONFIG_WIFI_2].button_stt = But_None;
        esp_event_post_to(btn_event_loop, BUTTON_EVENT_BASE, EVENT_BUTTON_CONFIG_WIFI, NULL, 0, portMAX_DELAY);
        return 1;
    }
#endif
    if(get_num_btn_keeping() == 1 && rd_exceed_us(time_start_hold, TIME_CYCLE_SET_PAIR)){
        for (uint8_t i = 0; i < NUM_ELEMENT; i++)
        {
            if (btn_is_keeping(i))
            {
                index_btn = i;
                clear_check_keep(i);
                esp_event_post_to(btn_event_loop, BUTTON_EVENT_BASE, EVENT_BUTTON_PAIR_K9B, &index_btn, 1, portMAX_DELAY);
#if NUM_ELEMENT == 1
                count_kick_out++;
                if (count_kick_out >= 3)
                {
                    count_kick_out = 0;
                    esp_event_post_to(btn_event_loop, BUTTON_EVENT_BASE, EVENT_BUTTON_KICK_OUT, NULL, 0, portMAX_DELAY);
                    return 3;
                }
                esp_event_post_to(btn_event_loop, BUTTON_EVENT_BASE, EVENT_BUTTON_INC_COUNT_KICK_OUT, NULL, 0, portMAX_DELAY);
#endif
                return 1;
            }
        }
    }
#if NUM_ELEMENT > 1
    else if(get_num_btn_keeping() == 2 && rd_exceed_us(time_start_hold, TIME_CYCLE_KICK_OUT)){
        for (size_t i = 0; i < NUM_ELEMENT; i++)
        {
            if (btn_is_keeping(i))
            {
                clear_check_keep(i);
            }
        }
        count_kick_out++;
        if (count_kick_out >= 3)
        {
            count_kick_out = 0;
            esp_event_post_to(btn_event_loop, BUTTON_EVENT_BASE, EVENT_BUTTON_KICK_OUT, NULL, 0, portMAX_DELAY);
            return 3;
        }
        esp_event_post_to(btn_event_loop, BUTTON_EVENT_BASE, EVENT_BUTTON_INC_COUNT_KICK_OUT, NULL, 0, portMAX_DELAY);
        return 1;
    }
#endif
    if (count_kick_out > 0 && rd_exceed_us(time_start_hold, TIME_OUT_KICK_OUT))
    {
        count_kick_out = 0;
        ESP_LOGW("BTN_MANAGER", "time out kick out, reset count");
        return 0;
    }    
    return -1;
}


void btn_event_init(esp_event_handler_t event_handler){
    esp_event_loop_args_t loop_args = {
        .queue_size = 5,
        .task_name = "btn_event_loop",
        .task_priority = uxTaskPriorityGet(NULL),
        .task_stack_size = 2048*3,
        .task_core_id = tskNO_AFFINITY
    };
    ESP_ERROR_CHECK(esp_event_loop_create(&loop_args, &btn_event_loop));
    esp_event_handler_register_with(btn_event_loop, BUTTON_EVENT_BASE, ESP_EVENT_ANY_ID, event_handler, NULL);
}

static void button_cb(void *args){
    scan_all_btn();
    btn_check_press();
    btn_check_keeping();
}

void button_manager_init(void)
{
    static esp_timer_handle_t g_button_timer_handle = NULL;
    if(!g_button_timer_handle){
        esp_timer_create_args_t button_timer = {0};
        button_timer.arg = NULL;
        button_timer.callback = button_cb;
        button_timer.dispatch_method = ESP_TIMER_TASK;
        button_timer.name = "button_timer";
        esp_timer_create(&button_timer, &g_button_timer_handle);
        esp_timer_start_periodic(g_button_timer_handle, TICK_INTERVAL * 1000U);
    }
}