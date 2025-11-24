#ifndef COMMON_H__
#define COMMON_H__

#include "LC8823.h"
#include "Define.h"
#include "esp_timer.h"

#define TIME_DELAY_KICK_OUT        3500*1000
#define TIMEOUT_SAVE_FLASH_CONFIG  3*1000*1000

struct on_off_t{
    uint8_t present;
    uint8_t target;
};

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
    int64_t time_now_s;
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

/**
 * @brief hàm kiểm tra và bắt đầu quá trình lưu cấu hình vào flash
 */
void start_check_save_flash(void);

/**
 * @brief hàm điều khiển on off 
 * 
 * @param index: chỉ số phần tử các element, bắt đầu từ 0
 * @param stt: trạng thái on off
 */
void control_set_onoff(uint8_t index, uint8_t stt);

/**
 * @brief hàm khởi động bộ đếm ngược
 * 
 * @param time_s: thời gian đếm ngược (giây)
 * @param target: trạng thái mục tiêu khi kết thúc đếm ngược 
 * @return esp_err_t: trả về kiểu esp_err_t
 */
esp_err_t rd_start_countdown(int64_t time_s, stt_target_coundown target);

/**
 * @brief hàm dừng bộ đếm timer cho countdown
 */
esp_err_t rd_stop_countdown(void);

/**
 * @brief hàm cài đặt trạng thái khởi động cho toàn bộ các element
 * 
 * @param stt_startup trạng thái khởi động
 */
void rd_set_status_startup(stt_startup_t stt_startup);

/**
 * @brief hàm lấy ra trạng thái nút nhấn hiện tại
 * 
 * @param index chỉ số phần tử
 * @return trả về trạng thái hiện tại của element index (ON_STT/ OFF_STT)
 */
uint8_t get_stt_present(uint8_t index);

/**
 * @brief hàm khởi tạo task controller
 */
void init_control_task(void);

/**
 * @brief hàm reset lại toàn bộ cấu hình thiết bị
 */
void reset_config_device(void);

#endif /*  */
