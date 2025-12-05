#include "rd_gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

gpio_num_t BUTTON_PIN_ARR[NUM_ELEMENT] = {
    #ifdef BTN_1
        BUTTON_PIN1
    #endif
    #ifdef BTN_2
        ,BUTTON_PIN2
    #endif
    #ifdef BTN_3
        ,BUTTON_PIN3
    #endif
    #ifdef BTN_4
        ,BUTTON_PIN4
    #endif
};
gpio_num_t RELAY_PIN_ARR[NUM_ELEMENT] = {
    #ifdef BTN_1
        RELAY1_PIN 
    #endif
    #ifdef BTN_2
        ,RELAY2_PIN
    #endif
    #ifdef BTN_3
        ,RELAY3_PIN
    #endif
    #ifdef BTN_4
        ,RELAY4_PIN
    #endif
};

static QueueHandle_t gpio_evt_queue = NULL;
void IRAM_ATTR gpio_isr_handler(void *arg) // send data from ISR to QUEUE
{
   uint32_t gpio_num = (uint32_t)arg;
   BaseType_t high_task_wakeup = pdFALSE;
   xQueueSendFromISR(gpio_evt_queue, &gpio_num, &high_task_wakeup);
   if(high_task_wakeup){
        portYIELD_FROM_ISR(); // Chuyển ngay sang task có priority cao hơn
   }
}

void gpio_set_pin_input(gpio_num_t GPIO_NUM, gpio_int_type_t INTR_TYPE)
{
    gpio_config_t gpio_config_pin = {
        .pin_bit_mask = 1ULL << GPIO_NUM,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = INTR_TYPE};
    gpio_config(&gpio_config_pin);
    if(INTR_TYPE != GPIO_INTR_DISABLE) gpio_evt_queue = xQueueCreate(30, sizeof(uint32_t));
}

void gpio_set_pin_output(gpio_num_t GPIO_NUM)
{
    gpio_config_t gpio_config_pin = {
        .pin_bit_mask = 1ULL << GPIO_NUM,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&gpio_config_pin);
}

void rd_gpio_init(void)
{
    for (uint8_t i = 0; i < NUM_ELEMENT; i++)
    {
        gpio_set_pin_input(BUTTON_PIN_ARR[i], GPIO_INTR_DISABLE);
        gpio_set_pin_output(RELAY_PIN_ARR[i]);
    }
    gpio_set_pin_output(LED_DATA);
    gpio_set_pin_output(LED_CLK);
    gpio_set_pin_output(RESET_TOUCH_PIN);

    gpio_set_level(RESET_TOUCH_PIN, TOUCH_ACTIVE_POW);
    gpio_set_level(LED_CLK, 1);
}

void test_gpio_init(){
  gpio_set_pin_output(RESET_TOUCH_PIN);
  gpio_set_level(RESET_TOUCH_PIN, TOUCH_ACTIVE_POW);

}
