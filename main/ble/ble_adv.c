#include "esp_log.h"
#include "esp_err.h"
#include "esp_bt.h"
#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"

#include "ble_adv.h"
#include "freertos/queue.h"

#define LOG_RAW_DATA 0

static const char *TAG = "BLE_ADV";
QueueHandle_t adv_queue_ble = NULL;
static bool is_init_ble = false;

void rd_init_queue_adv(void)
{
    adv_queue_ble = xQueueCreate(30, sizeof(rd_ble_adv_t));
    if (adv_queue_ble == NULL)
    {
        ESP_LOGE(TAG, "Failed to create queue adv");
    }
}

static int ble_scan_cb(struct ble_gap_event *event, void *arg)
{
    switch (event->type)
    {
    case BLE_GAP_EVENT_DISC:
    {
        // const struct ble_hs_adv_fields *fields = &event->disc.fields;
        const ble_addr_t *addr = &event->disc.addr;
        int rssi = event->disc.rssi;
        uint8_t adv_len = event->disc.length_data;
        const uint8_t *adv_data = event->disc.data;

        if (adv_len == 15 && adv_data[0] == 0x0E && adv_data[1] == 0xff)
        {
#if LOG_RAW_DATA
            printf("ADV: RSSI=%d, len=%d, addr=%02X:%02X:%02X:%02X:%02X:%02X\n",
                   rssi, adv_len,
                   addr->val[5], addr->val[4], addr->val[3],
                   addr->val[2], addr->val[1], addr->val[0]);

            for (int i = 0; i < adv_len; i++)
            {
                printf("%02X ", adv_data[i]);
            }
            printf("\n\n");
#endif
            rd_ble_adv_t rd_mess_adv = {0};
            rd_mess_adv.rssi = rssi;
            memcpy(rd_mess_adv.mac, &addr->val[0], 6);
            memcpy(rd_mess_adv.buf, &adv_data[0], adv_len);
            if (xQueueSend(adv_queue_ble, &rd_mess_adv, 0) != pdTRUE)
            {
                ESP_LOGE(TAG, "Queue full or send failed");
            }
        }

        return 0;
    }
    default:
        return 0;
    }
}

void ble_app_on_sync(void)
{
    struct ble_gap_disc_params disc_params = {0};
    disc_params.itvl = 0x0010;
    disc_params.window = 0x0010;
    disc_params.filter_policy = 0;
    disc_params.passive = 0;
    disc_params.filter_duplicates = 0;

    ESP_LOGI("BLE_ADV", "Starting BLE scan...");
    ble_gap_disc(0, BLE_HS_FOREVER, &disc_params, ble_scan_cb, NULL);
}

void ble_host_task(void *param)
{
    nimble_port_run(); // BLE host loop
    nimble_port_freertos_deinit();
    vTaskDelete(NULL);  
}

void deinit_ble_task(void)
{
    if (is_init_ble)
    {
        ESP_LOGW(TAG, "deinit ble adv");
        is_init_ble = false;
        nimble_port_stop();
        vTaskDelay(pdMS_TO_TICKS(100));
        ESP_ERROR_CHECK(nimble_port_deinit());
    }
}

void init_ble_adv(void)
{
    // esp_err_t ret;
    if (!is_init_ble)
    {
        ESP_LOGW(TAG, "init ble adv RD");
        is_init_ble = true;
        if (adv_queue_ble == NULL)
        {
            rd_init_queue_adv();
        }
        else
        {
            ESP_LOGW(TAG, "queue init done!");
        }
        ESP_ERROR_CHECK(nimble_port_init());
        ble_hs_cfg.sync_cb = ble_app_on_sync;
        nimble_port_freertos_init(ble_host_task);
    }
}