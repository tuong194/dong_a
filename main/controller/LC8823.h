#ifndef _LC_8823_H__
#define _LC_8823_H__

#include "stdint.h"
#include "stdbool.h"

#define LIGHT_SET(dim_val)  (0xE0 | (dim_val))
#define LIGHT_MAX    0xEF  //50%
#define LIGHT_MIN    0xE2  //2/31 = 6%
#define EN_COLOR     0xff
#define DIS_COLOR    0x00

#define FREQ_LED 25000

#define START_FRAME  0x00
#define END_FRAME    0xff

#define min2(a,b)	 ((a) < (b) ? (a) : (b))

enum rd_led
{
    LED_1 = 0,   //-> ele 0
    LED_4,       //-> ele 1
    LED_2,
    LED_3,
    LED_WIFI
};

struct led_data_t{
    uint8_t lum;
    uint8_t blue;
    uint8_t green;
    uint8_t red;
};
struct led_sigle_blink_t
{
	uint8_t num_of_cycle;
	uint32_t time_cycle_ms;
	int64_t last_clockTime_toggle_ms;
};

typedef struct{
    struct led_data_t ON;
    struct led_data_t OFF;
}led_value_onoff_t;

uint8_t get_dim_onoff_current(uint8_t index, uint8_t stt);
uint8_t get_red_color_current(uint8_t index, uint8_t stt);
uint8_t get_green_color_current(uint8_t index, uint8_t stt);
uint8_t get_blue_color_current(uint8_t index, uint8_t stt);

void lc8823_set_stt_led(uint8_t index, uint8_t stt);
void lc8823_load_value_dim_onoff_from_flash(uint8_t index, uint8_t onoff, uint8_t dim_value);
void lc8823_set_dim_onoff(uint8_t index, uint8_t onoff, uint8_t dim_value);
void lc8823_set_color(uint8_t index, uint8_t onoff, uint8_t red, uint8_t green, uint8_t blue);
void lc8823_update_color(uint8_t index, uint8_t stt);
void led_set_blink(uint8_t led_id, uint8_t num_cycle, uint32_t time_tongle_ms);
void lc8823_blink_led(uint8_t led_idx, uint8_t num_cycle, uint16_t time_delay_ms);
void config_stt_connect_mqtt(bool stt);
void led_manager_init();
void led_manager_loop();

#endif /*  */
