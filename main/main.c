#include "esp_log.h"
#include "ble_adv.h"
#include "K9B_Remote.h"
#include "rd_common.h"
#include "rd_flash.h"
#include "wifi_prov_mgr.h"
#include "tb.h"
#define TAG "Main"


extern void Config_Read_Flash();
void app_main(void)
{
	ESP_LOGW("MAIN", "=============================firm version %s=============================",PROJECT_VER);
	rd_flash_init();
	Config_Read_Flash();
	init_ble_adv();
	K9B_init_task();
	init_control_task();
	init_wifi_provision();
	tb_init();
}
