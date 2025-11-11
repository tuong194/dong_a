
#include "rd_ble.h"
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_bt.h"

#define TAG "RD_BLE"
esp_err_t rd_bluetooth_init(void)
{
    esp_err_t ret = 0;
    printf("enable bt_controller=========\n\n");
    if (esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_IDLE)
    {
        esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
        // esp_bt_controller_init(&bt_cfg);
    }
    if (esp_bt_controller_get_status() != ESP_BT_CONTROLLER_STATUS_ENABLED)
    {
        esp_bt_controller_enable(ESP_BT_MODE_BLE);
    }
    // esp_nimble_hci_and_controller_init();
    // nimble_port_init();
    return ret;
}
void rd_dis_bluetooth()
{
    
// if (nimble_port_get_dflt_eventq() != NULL) {
//     nimble_port_stop();
//     nimble_port_deinit();
// }
    // esp_nimble_hci_and_controller_deinit();
    if (esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_ENABLED)
    {
        printf("dis ble =========\n\n\n");
        esp_bt_controller_disable();  // dis radio ble
    }
    if (esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_INITED)
    {
        // esp_bt_controller_deinit(); //release driver + HW 
    }
    // esp_bt_mem_release(ESP_BT_MODE_BTDM);
}