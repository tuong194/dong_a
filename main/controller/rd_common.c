#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"

#include "rd_gpio.h"
#include "button.h"
#include "relay.h"
#include "rd_common.h"
#include "device.h"
#include "K9B_Remote.h"
#include "rd_flash.h"
#include "util.h"
#include "wifi_prov_mgr.h"
#include "err_code.h"

static void btn_event_handle(void* arg, esp_event_base_t event_base, int32_t event_id, void* data);

struct on_off_t on_off_val[NUM_ELEMENT];
static Countdown_t countdown_val[NUM_ELEMENT] = {0};
static flash_config_value_t flash_config_value = {0};

save_flash_check_t flash_check_save = {
    .is_save_flash = false,
    .time_start_save_flash = 0,
};


static void common_init(void)
{
    rd_gpio_init();
    led_manager_init();
    btn_event_init(btn_event_handle);
}

static void init_value_config_default(void)
{
    flash_config_value.header[0] = FLASH_HEADER_1;
    flash_config_value.header[1] = FLASH_HEADER_2;
    flash_config_value.header[2] = FLASH_HEADER_1;
    flash_config_value.header[3] = FLASH_HEADER_2;
    flash_config_value.stt_startup = START_RESTORE;

    for (uint8_t i = 0; i < NUM_ELEMENT; i++)
    {
        flash_config_value.on_off[i] = ON_STT;
        flash_config_value.led_value[i].ON.lum = LIGHT_MAX;
        flash_config_value.led_value[i].ON.blue = 0xb2;
        flash_config_value.led_value[i].ON.green = 0xff;
        flash_config_value.led_value[i].ON.red = 0x00;

        flash_config_value.led_value[i].OFF.lum = LIGHT_MIN;
        flash_config_value.led_value[i].OFF.blue = 0xb2;
        flash_config_value.led_value[i].OFF.green = 0xff;
        flash_config_value.led_value[i].OFF.red = 0x00;
    }
    rd_write_flash(KEY_FLASH_CONFIG, &flash_config_value, sizeof(flash_config_value));
}

static int load_data_from_flash(void){
    for (uint8_t i = 0; i < NUM_ELEMENT; i++)
    {
        switch (flash_config_value.stt_startup)
        {
        case START_OFF:
            on_off_val[i].present = OFF_STT;
            on_off_val[i].target = OFF_STT;
            break;
        case START_ON:
            on_off_val[i].present = ON_STT;
            on_off_val[i].target = ON_STT;
            break;
        case START_RESTORE:
            on_off_val[i].present = flash_config_value.on_off[i];
            on_off_val[i].target = flash_config_value.on_off[i];
            break;    
        default:
            on_off_val[i].present = flash_config_value.on_off[i];
            on_off_val[i].target = flash_config_value.on_off[i];
            break;
        }
        lc8823_load_value_dim_onoff_from_flash(i, ON_STT, flash_config_value.led_value[i].ON.lum); //dim ON
        lc8823_load_value_dim_onoff_from_flash(i, OFF_STT, flash_config_value.led_value[i].OFF.lum); // dim OFF
        lc8823_set_color(i, ON_STT, flash_config_value.led_value[i].ON.red, flash_config_value.led_value[i].ON.green, flash_config_value.led_value[i].ON.blue); //rgb on
        lc8823_set_color(i, OFF_STT, flash_config_value.led_value[i].OFF.red, flash_config_value.led_value[i].OFF.green, flash_config_value.led_value[i].OFF.blue); //rgb on

        lc8823_set_stt_led(i, on_off_val[i].present); // set stt led
        set_stt_relay(i, on_off_val[i].present);
    }
    return CODE_OK;
}

int init_flash_config(void){
    rd_read_flash(KEY_FLASH_CONFIG, &flash_config_value, sizeof(flash_config_value));
    // printf("flash config header : %02x-%02x-%02x-%02x\n", flash_config_value.header[0], flash_config_value.header[1], flash_config_value.header[2], flash_config_value.header[3]);
    // printf("ON.lum: %2x, Off lum: %2x\n", flash_config_value.led_value[0].ON.lum, flash_config_value.led_value[0].OFF.lum);
    if (flash_config_value.header[0] != FLASH_HEADER_1 && flash_config_value.header[1] != FLASH_HEADER_2 &&\
        flash_config_value.header[2] != FLASH_HEADER_1 && flash_config_value.header[3] != FLASH_HEADER_2)
    {
        init_value_config_default();
        load_data_from_flash();
        return CODE_FLASH_ERR;
    }
    load_data_from_flash();
    return CODE_OK;
}

/*========================================SAVE flash after 3 second========================================*/
void start_check_save_flash(void)
{
    flash_check_save.is_save_flash = true;
    flash_check_save.time_start_save_flash = esp_timer_get_time();
}

int loop_check_save_flash(void){
    if (flash_check_save.is_save_flash)
    {
        if (rd_exceed_us(flash_check_save.time_start_save_flash, TIMEOUT_SAVE_FLASH_CONFIG))
        {
            flash_check_save.is_save_flash = false;
            flash_check_save.time_start_save_flash = 0;

            for (uint8_t i = 0; i < NUM_ELEMENT; i++)
            {
                flash_config_value.on_off[i] = on_off_val[i].present;
                flash_config_value.led_value[i].ON.lum = get_dim_onoff_current(i, ON_STT);
                flash_config_value.led_value[i].ON.blue = get_blue_color_current(i, ON_STT);
                flash_config_value.led_value[i].ON.green = get_green_color_current(i, ON_STT);
                flash_config_value.led_value[i].ON.red = get_red_color_current(i, ON_STT);

                flash_config_value.led_value[i].OFF.lum = get_dim_onoff_current(i, OFF_STT);;
                flash_config_value.led_value[i].OFF.blue = get_blue_color_current(i, OFF_STT);;
                flash_config_value.led_value[i].OFF.green = get_green_color_current(i, OFF_STT);
                flash_config_value.led_value[i].OFF.red = get_red_color_current(i, OFF_STT);
            }
            rd_write_flash(KEY_FLASH_CONFIG, &flash_config_value, sizeof(flash_config_value));
            printf("save flash config\n");
            return CODE_OK;
        }
    }
    return CODE_ERR;
}


void init_onoff_value(void)
{
    for (uint8_t i = 0; i < NUM_ELEMENT; i++)
    {
        on_off_val[i].present = ON_STT;
        on_off_val[i].target = ON_STT;

        lc8823_set_stt_led(i, on_off_val[i].present); // set stt led
        set_stt_relay(i, on_off_val[i].present);
    }
}

void control_on_off_scan(void)
{
    for (uint8_t i = 0; i < NUM_ELEMENT; i++)
    {
        if (on_off_val[i].present != on_off_val[i].target)
        {
            start_check_save_flash();
            on_off_val[i].present = on_off_val[i].target;
            lc8823_set_stt_led(i, on_off_val[i].present);
            set_stt_relay(i, on_off_val[i].present);
            post_stt_btn_server(i, on_off_val[i].present); // post server
        }
    }
}

void control_set_onoff(uint8_t index, uint8_t stt)
{
    on_off_val[index].target = stt;
}

uint8_t get_stt_present(uint8_t index)
{
    return on_off_val[index].present;
}

void rd_set_countdown(uint8_t index, int64_t time_s, stt_target_coundown target)
{
    ESP_LOGI("COMMON", "set countdown: %lld second, index: %02x", time_s, index);
    if (index == 0xff)
    {
        for (uint8_t i = 0; i < NUM_ELEMENT; i++)
        {
            printf("set countdown: %lld second, index: %02x \n", time_s, i);
            countdown_val[i].stt_target = target;
            countdown_val[i].time_countdown_s = time_s*1000*1000;
            countdown_val[i].time_start_countdown = esp_timer_get_time();
        }
        return;
    }
    countdown_val[index].stt_target = target;
    countdown_val[index].time_countdown_s = time_s*1000*1000;
    countdown_val[index].time_start_countdown = esp_timer_get_time();
}

void rd_set_status_startup(stt_startup_t stt_startup){
    for (uint8_t i = 0; i < NUM_ELEMENT; i++)
    {
        switch (stt_startup)
        {
        case START_OFF:
            control_set_onoff(i, 0);
            break;
        case START_ON:
            control_set_onoff(i, 1);
            break;
        case START_RESTORE:
            control_set_onoff(i, 0);
            break;
        default:
            break;
        }
    }
}

static void loop_check_countdown(void)
{
    for (uint8_t i = 0; i < NUM_ELEMENT; i++)
    {
        if (countdown_val[i].time_countdown_s != 0)
        {
            if (esp_timer_get_time() - countdown_val[i].time_start_countdown >= countdown_val[i].time_countdown_s)
            {
                printf("set stt onoff countdown element : %x\n", i);
                switch (countdown_val[i].stt_target)
                {
                case OFF:
                    control_set_onoff(i, 0);
                    break;
                case ON:
                    control_set_onoff(i, 1);
                    break;
                case TONGLE:
                    control_set_onoff(i, !on_off_val[i].present);
                    break;
                default:
                    break;
                }
                countdown_val[i].stt_target = OFF;
                countdown_val[i].time_countdown_s = 0;
                countdown_val[i].time_start_countdown = 0;
            }
        }
    }
}

static void btn_event_handle(void* arg, esp_event_base_t event_base, int32_t event_id, void* data){
    if(event_base == BUTTON_EVENT_BASE){
        switch (event_id)
        {
        case EVENT_BUTTON_PRESS:{
            uint8_t index = *(uint8_t *)data;
            uint8_t stt = !get_stt_present(index);
            control_set_onoff(index, stt);
            break;
        }
        case EVENT_BUTTON_PAIR_K9B:{
            uint8_t index = *(uint8_t *)data;
            led_set_blink(index, 5, 300);
            K9B_set_start_pair_onoff(index);
            break;
        }
        case EVENT_BUTTON_DELETE_ALL_K9B:{
            uint8_t index = *(uint8_t *)data;
            K9B_delete_all_onoff_one_btn(index);
            K9B_Pair_OnOff_ClearFlag();
            led_set_blink(index, 11, 300);
            break;
        }
        case EVENT_BUTTON_CONFIG_WIFI:
            ESP_LOGI("COMMON","event config wwifi");
            led_set_blink(ALL_LED, 7, 100);
            start_wifi_prov_mgr();
            break;
        case EVENT_BUTTON_KICK_OUT:
            ESP_LOGI("BTN_MANAGER", "KICK OUT !!!");
            lc8823_blink_led(ALL_LED, 11, 300);
            reset_wifi_prov_mgr();
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            esp_restart();
            break;
        case EVENT_BUTTON_INC_COUNT_KICK_OUT:
            ESP_LOGI("COMMON","event increase kick out");
            led_set_blink(ALL_LED, 3, 300);
            break;        
        default:
            ESP_LOGE("COMMON","UNKNOW EVENT_ID BUTTON_EVENT_BASE");
            break;
        }
    }
}

static void control_task(void *arg)
{
    common_init();
    init_flash_config();
    

    while (1)
    {
        led_manager_loop();
        btn_manager_loop();
        relay_manager_loop();

        control_on_off_scan();

        K9B_loop_check_pair_time_out();
        loop_check_countdown();
        loop_check_save_flash();

        vTaskDelay(TICK_INTERVAL / portTICK_PERIOD_MS);
    }
}

void init_control_task(void)
{
    xTaskCreate(control_task, "control_task", 2048 * 2, NULL, configMAX_PRIORITIES - 3, NULL);
}
