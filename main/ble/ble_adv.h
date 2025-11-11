
#ifndef _BLE_ADV_H__
#define _BLE_ADV_H__

#include "stdint.h"

typedef struct __attribute__((packed)){
    uint8_t mac[6];
    int8_t rssi;
    uint8_t buf[15];
}rd_ble_adv_t;

void rd_init_queue_adv(void);
void init_ble_adv(void);
void deinit_ble_task(void);

#endif