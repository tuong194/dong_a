#include "LC8823.h"
#include "Define.h"
#include "util.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"

#define LED_DATA_SET(bit_send)   gpio_set_level(LED_DATA, bit_send)
#define LED_CLK_HIGH()           gpio_set_level(LED_CLK,1)
#define LED_CLK_LOW()            gpio_set_level(LED_CLK,0)

static struct led_data_t led_data[NUM_ELEMENT + 1];
static struct led_sigle_blink_t led_blink_val[NUM_ELEMENT] = {0};
led_value_onoff_t led_value[NUM_ELEMENT + 1];

extern uint8_t get_stt_present(uint8_t index);
static void led_update_stt(void);

void LC8823_send_bit(uint8_t bit_send)
{
    LED_CLK_LOW();
    SLEEP_US(500000 / FREQ_LED);
    LED_DATA_SET(bit_send);
    LED_CLK_HIGH();
    SLEEP_US(500000 / FREQ_LED);
}
void LC8823_send_byte(uint8_t byte_send)
{
    uint8_t bit_check;
    for (int8_t i = 7; i >= 0; i--)
    {
        bit_check = (byte_send >> i) & 0x01;
        LC8823_send_bit(bit_check);
    }
}
void start_frame_led(void)
{
    LC8823_send_byte(START_FRAME);
    LC8823_send_byte(START_FRAME);
    LC8823_send_byte(START_FRAME);
    LC8823_send_byte(START_FRAME);
}
void end_frame_led(void)
{
    LC8823_send_byte(END_FRAME);
    LC8823_send_byte(END_FRAME);
    LC8823_send_byte(END_FRAME);
    LC8823_send_byte(END_FRAME);
    LED_CLK_HIGH();
}

void lc8823_set_led_data(uint8_t lightness, uint8_t blue, uint8_t green, uint8_t red)
{
    LC8823_send_byte(lightness);
    LC8823_send_byte(blue);
    LC8823_send_byte(green);
    LC8823_send_byte(red);
}

void lc8823_load_value_dim_onoff_from_flash(uint8_t index, uint8_t onoff, uint8_t dim_value){
    if (onoff)
    {
        led_value[index].ON.lum = dim_value;
    }
    else
    {
        led_value[index].OFF.lum = dim_value;
    }
    uint8_t stt_onoff = get_stt_present(index);
    led_data[index].lum = stt_onoff ? led_value[index].ON.lum : led_value[index].OFF.lum;
}

void lc8823_set_dim_onoff(uint8_t index, uint8_t onoff, uint8_t dim_value)
{
    dim_value = dim_value * 31 / 100;
    ESP_LOGW("LC8823", "set dim: %02X", dim_value);
    if (onoff)
    {
        led_value[index].ON.lum = LIGHT_SET(dim_value);
    }
    else
    {
        led_value[index].OFF.lum = LIGHT_SET(dim_value);
    }
    uint8_t stt_onoff = get_stt_present(index);
    led_data[index].lum = stt_onoff ? led_value[index].ON.lum : led_value[index].OFF.lum;
    led_update_stt();
}

void lc8823_set_stt_led(uint8_t index, uint8_t stt)
{
    led_data[index].lum = stt ? led_value[index].ON.lum : led_value[index].OFF.lum; // update stt
    lc8823_update_color(index, stt); //update color
    // led_update_stt();
}

void lc8823_update_color(uint8_t index, uint8_t stt)
{
    if (stt)
    {
        led_data[index].blue = led_value[index].ON.blue;
        led_data[index].green = led_value[index].ON.green;
        led_data[index].red = led_value[index].ON.red;
    }
    else
    {
        led_data[index].blue = led_value[index].OFF.blue;
        led_data[index].green = led_value[index].OFF.green;
        led_data[index].red = led_value[index].OFF.red;
    }
    led_update_stt();
}
void lc8823_set_color(uint8_t index, uint8_t onoff, uint8_t red, uint8_t green, uint8_t blue)
{   
    if (onoff)
    {
        led_value[index].ON.blue = blue;
        led_value[index].ON.green = green;
        led_value[index].ON.red = red;
    }
    else
    {
        led_value[index].OFF.blue = blue;
        led_value[index].OFF.green = green;
        led_value[index].OFF.red = red;
    }
    uint8_t stt_onoff = get_stt_present(index);
    lc8823_update_color(index, stt_onoff);
    // led_update_stt();
}

uint8_t get_dim_onoff_current(uint8_t index, uint8_t stt){
    return stt ? led_value[index].ON.lum : led_value[index].OFF.lum;
}

uint8_t get_red_color_current(uint8_t index, uint8_t stt){
    return stt ? led_value[index].ON.red : led_value[index].OFF.red;
}
uint8_t get_green_color_current(uint8_t index, uint8_t stt){
    return stt ? led_value[index].ON.green : led_value[index].OFF.green;
}
uint8_t get_blue_color_current(uint8_t index, uint8_t stt){
    return stt ? led_value[index].ON.blue : led_value[index].OFF.blue;
}

void led_set_blink(uint8_t led_id, uint8_t num_cycle, uint32_t time_tongle_ms)
{
    if (led_id == 0xff)
    {
        for (uint8_t i = 0; i < NUM_ELEMENT; i++)
        {
            led_blink_val[i].num_of_cycle = num_cycle;
            led_blink_val[i].time_cycle_ms = time_tongle_ms * 1000;
        }
    }
    else
    {
        led_id = min2(led_id, NUM_ELEMENT - 1);
        led_blink_val[led_id].num_of_cycle = num_cycle;
        led_blink_val[led_id].time_cycle_ms = time_tongle_ms * 1000;
    }
}

void led_reload_data(uint8_t index){
    uint8_t stt = get_stt_present(index);
    lc8823_set_stt_led(index, stt);
}

void scan_blink_led(void)
{
    for (uint8_t i = 0; i < NUM_ELEMENT; i++)
    {
        if (led_blink_val[i].num_of_cycle > 0)
        {
            if (rd_exceed_us(led_blink_val[i].last_clockTime_toggle_ms, led_blink_val[i].time_cycle_ms))
            {
                if (led_blink_val[i].num_of_cycle == 1)
                {
                    led_reload_data(i);
                    led_blink_val[i].num_of_cycle--;
                    return;
                }
                if (led_blink_val[i].num_of_cycle % 2 == 0)
                {
                    lc8823_set_stt_led(i,1); // dim high
                }
                else
                {
                    lc8823_set_stt_led(i,0); // dim low
                }
                led_blink_val[i].num_of_cycle--;
                led_blink_val[i].last_clockTime_toggle_ms = esp_timer_get_time();
            }
        }
    }
}

void config_stt_connect_mqtt(bool stt){
    if(stt){
        lc8823_set_color(LED_WIFI, 0, 0, 0xff, 0);
    }else{
        lc8823_set_color(LED_WIFI, 0, 0xff,0,0);
    }
    lc8823_update_color(LED_WIFI, 0);
    // led_update_stt();
}

static void led_update_stt(void){
    start_frame_led();

    lc8823_set_led_data(led_data[LED_1].lum, led_data[LED_1].blue, led_data[LED_1].green, led_data[LED_1].red);
    lc8823_set_led_data(led_data[LED_1].lum, led_data[LED_1].blue, led_data[LED_1].green, led_data[LED_1].red);

    lc8823_set_led_data(led_data[LED_2].lum, led_data[LED_2].blue, led_data[LED_2].green, led_data[LED_2].red);
    lc8823_set_led_data(led_data[LED_2].lum, led_data[LED_2].blue, led_data[LED_2].green, led_data[LED_2].red);

    lc8823_set_led_data(led_data[LED_3].lum, led_data[LED_3].blue, led_data[LED_3].green, led_data[LED_3].red);
    lc8823_set_led_data(led_data[LED_3].lum, led_data[LED_3].blue, led_data[LED_3].green, led_data[LED_3].red);

    lc8823_set_led_data(led_data[LED_4].lum, led_data[LED_4].blue, led_data[LED_4].green, led_data[LED_4].red);
    lc8823_set_led_data(led_data[LED_4].lum, led_data[LED_4].blue, led_data[LED_4].green, led_data[LED_4].red);


    lc8823_set_led_data(led_data[LED_WIFI].lum, led_data[LED_WIFI].blue, led_data[LED_WIFI].green, led_data[LED_WIFI].red);
    end_frame_led();
}

#if 1
void lc8823_blink_led(uint8_t led_idx, uint8_t num_cycle, uint16_t time_delay_ms){
    if(led_idx == 0xff){
        while(num_cycle > 0){
            for (uint8_t i = 0; i < NUM_ELEMENT; i++){
                if(num_cycle == 1){
                    led_reload_data(i);
                    return;
                }
                if(num_cycle % 2 == 0){
                    lc8823_set_stt_led(i,1);
                }else{
                    lc8823_set_stt_led(i,0);
                }
            }
            num_cycle--;
            SLEEP_US(time_delay_ms*1000);
        }
    }
}
#endif

void init_value_default(void){
    for (uint8_t i = 0; i < NUM_ELEMENT; i++)
    {
        led_value[i].ON.lum = LIGHT_MAX;
        led_value[i].ON.blue = 0xb2;
        led_value[i].ON.green = 0xff;
        led_value[i].ON.red = 0x00;

        led_value[i].OFF.lum = LIGHT_MIN;
        led_value[i].OFF.blue = 0xb2;
        led_value[i].OFF.green = 0xff;
        led_value[i].OFF.red = 0x00;
    }
    led_value[LED_WIFI].ON.lum = LIGHT_MIN;
    led_value[LED_WIFI].ON.blue = 0x00;
    led_value[LED_WIFI].ON.green = 0x00;
    led_value[LED_WIFI].ON.red = 0xff;

    led_value[LED_WIFI].OFF.lum = LIGHT_MIN;
    led_value[LED_WIFI].OFF.blue = 0x00;
    led_value[LED_WIFI].OFF.green = 0x00;
    led_value[LED_WIFI].OFF.red = 0xff;
}

void init_led(void){
    for (uint8_t i = 0; i < NUM_ELEMENT+1; i++){
        led_data[i].lum = led_value[i].OFF.lum;
        led_data[i].blue = led_value[i].OFF.blue;
        led_data[i].green = led_value[i].OFF.green;
        led_data[i].red = led_value[i].OFF.red;
    }
    led_data[LED_WIFI].lum = led_value[LED_WIFI].OFF.lum; // init led wifi
    led_update_stt();
}



void led_manager_init(){
    init_value_default();
    init_led();
}

void led_manager_loop(){
    scan_blink_led();
}