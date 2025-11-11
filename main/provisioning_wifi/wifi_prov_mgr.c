#include <stdio.h>
#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#include "esp_err.h"
#include <esp_log.h>
#include "esp_timer.h"
#include <esp_wifi.h>
#include <esp_event.h>
#include <nvs_flash.h>
#include <wifi_provisioning/manager.h>
#include <wifi_provisioning/scheme_ble.h>
#include <wifi_provisioning/scheme_softap.h>

#include "wifi_prov_mgr.h"
#include "ble_adv.h"

// ESP_EVENT_DEFINE_BASE(RD_WIFI_EVENT_BASE);

static const char *TAG = "RD_WIFI_PROV_MGR";
const int WIFI_CONNECTED_EVENT = BIT0;
static EventGroupHandle_t wifi_event_group;

static void config_wifi_prov(type_transport_e type_prov);
static void time_out_prov_task(void *param);

TaskHandle_t TaskCheckTimeOut = NULL;
static config_wifi_t config_wifi = {
    .is_config_wifi = false,
    .is_provisioning = false,
    .time_out = 0,
    .type_prov = TRANSPORT_SOFTAP
};
uint8_t ssid_config[32];
uint8_t pass_config[64];

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
#ifdef RD_RESET_PROV_MGR_ON_FAILURE
    static int retries;
#endif
    if (event_base == WIFI_PROV_EVENT)
    {
        switch (event_id)
        {
        case WIFI_PROV_START:
            ESP_LOGI(TAG, "Provisioning started");
            break;
        case WIFI_PROV_CRED_RECV:
        {
            // flag_start_config = false; // RD_NOTE đã nhập SSID, PASSWORD
            wifi_sta_config_t *wifi_sta_cfg = (wifi_sta_config_t *)event_data;
            ESP_LOGI(TAG, "Received Wi-Fi credentials"
                          "\n\tSSID     : %s\n\tPassword : %s",
                     (const char *)wifi_sta_cfg->ssid,
                     (const char *)wifi_sta_cfg->password);
            break;
        }
        case WIFI_PROV_CRED_FAIL:
        {
            wifi_prov_sta_fail_reason_t *reason = (wifi_prov_sta_fail_reason_t *)event_data;
            ESP_LOGE(TAG, "Provisioning failed!\n\tReason : %s"
                          "\n\tPlease reset to factory and retry provisioning",
                     (*reason == WIFI_PROV_STA_AUTH_ERROR) ? "Wi-Fi station authentication failed" : "Wi-Fi access-point not found");
#ifdef RD_RESET_PROV_MGR_ON_FAILURE
            retries++;
            if (retries >= PROV_MGR_MAX_RETRY_CNT)
            {
                // RD_NOTE prov FAIL
                ESP_LOGE(TAG, "Failed to connect with provisioned AP, reseting provisioned credentials");
                wifi_prov_mgr_reset_sm_state_on_failure(); // reset state machine
                retries = 0;
            }
#endif
            break;
        }
        case WIFI_PROV_CRED_SUCCESS:
            ESP_LOGI(TAG, "Provisioning successful");
#ifdef RD_RESET_PROV_MGR_ON_FAILURE
            retries = 0;
#endif
            break;
        case WIFI_PROV_END:
            /* De-initialize manager once provisioning is finished */
            ESP_LOGW("RD WIFI PROV", "\n wifi provision end, deinit wifi_prov_mgr\n");
            wifi_prov_mgr_deinit();
            config_wifi.is_provisioning = false;
            if(config_wifi.is_config_wifi){
                config_wifi.is_config_wifi = false;
                config_wifi_prov(config_wifi.type_prov);
            }
            init_ble_adv();
            break;
        default:
            break;
        }
    }
    else if (event_base == WIFI_EVENT)
    {
        switch (event_id)
        {
        case WIFI_EVENT_STA_START:
        {
            wifi_config_t sta_cfg = {0};
            ESP_ERROR_CHECK(esp_wifi_get_config(WIFI_IF_STA, &sta_cfg));
            ESP_LOGW(TAG, "SSID: %s", sta_cfg.sta.ssid);
            ESP_LOGW(TAG, "Password: %s", sta_cfg.sta.password);
            // sprintf(config.ssid_config, "%s", sta_cfg.sta.ssid);
            // memcpy(ssid_config, sta_cfg.sta.ssid, sizeof(sta_cfg.sta.ssid));
            // memcpy(pass_config, sta_cfg.sta.password, sizeof(sta_cfg.sta.password));

            esp_wifi_connect();
            break;
        }
        case WIFI_EVENT_STA_DISCONNECTED:
        {
            ESP_LOGI(TAG, "Disconnected. Connecting to the AP again...");
            esp_wifi_connect();
            break;
        }
            // #if RD_CONFIG_PROV_TRANSPORT == RD_PROV_TRANSPORT_SOFTAP
        case WIFI_EVENT_AP_STACONNECTED:
        {
            ESP_LOGI(TAG, "SoftAP transport: Connected!");
            break;
        }
        case WIFI_EVENT_AP_STADISCONNECTED:
        {
            ESP_LOGI(TAG, "SoftAP transport: Disconnected!");
            break;
        }
            // #endif
        default:
            break;
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        // uint32_t ip_connect = (uint32_t)event->ip_info.ip;
        ESP_LOGI(TAG, "Connected with IP Address:" IPSTR, IP2STR(&event->ip_info.ip));
        // sprintf((char *)config.ip, (const char *)"%d.%d.%d.%d", (ip_connect>>24) & 0xff, (ip_connect>>16) & 0xff, (ip_connect>>8) & 0xff, (ip_connect) & 0xff);
        // sprintf(config.ip, "%d.%d.%d.%d", IP2STR(&event->ip_info.ip));
        // printf("ip: %s\n\n", config.ip);
        /* Signal main application to continue execution */
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_EVENT);
        // #if RD_CONFIG_PROV_TRANSPORT == RD_PROV_TRANSPORT_BLE
    }
    else if (event_base == PROTOCOMM_TRANSPORT_BLE_EVENT)
    {
        switch (event_id)
        {
        case PROTOCOMM_TRANSPORT_BLE_CONNECTED:
            ESP_LOGI(TAG, "BLE transport: Connected!");
            break;
        case PROTOCOMM_TRANSPORT_BLE_DISCONNECTED:
            ESP_LOGI(TAG, "BLE transport: Disconnected!");
            break;
        default:
            break;
        }
        // #endif
    }
    else if (event_base == PROTOCOMM_SECURITY_SESSION_EVENT)
    {
        switch (event_id)
        {
        case PROTOCOMM_SECURITY_SESSION_SETUP_OK:
            ESP_LOGI(TAG, "Secured session established!");
            break;
        case PROTOCOMM_SECURITY_SESSION_INVALID_SECURITY_PARAMS:
            ESP_LOGE(TAG, "Received invalid security parameters for establishing secure session!");
            break;
        case PROTOCOMM_SECURITY_SESSION_CREDENTIALS_MISMATCH:
            ESP_LOGE(TAG, "Received incorrect username and/or PoP for establishing secure session!");
            break;
        default:
            break;
        }
    }
}

void init_wifi_prov_mgr(void)
{
    /* Initialize TCP/IP */
    ESP_ERROR_CHECK(esp_netif_init());
    /* Initialize the event loop */
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_event_group = xEventGroupCreate();

    /* Register our event handler for Wi-Fi, IP and Provisioning related events */
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_PROV_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    // #if RD_CONFIG_PROV_TRANSPORT == RD_PROV_TRANSPORT_BLE
    ESP_ERROR_CHECK(esp_event_handler_register(PROTOCOMM_TRANSPORT_BLE_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    // #endif
    ESP_ERROR_CHECK(esp_event_handler_register(PROTOCOMM_SECURITY_SESSION_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    esp_netif_create_default_wifi_sta();
    esp_netif_create_default_wifi_ap();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
}

static void wifi_init_sta_mode(void)
{
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

static void get_device_service_name(char *service_name, size_t max)
{
    uint8_t eth_mac[6];
    const char *ssid_prefix = RD_HEADER_SSID_WIFI_SOFTAP;
    esp_wifi_get_mac(WIFI_IF_STA, eth_mac);
    snprintf(service_name, max, "%s%02X%02X%02X",
             ssid_prefix, eth_mac[3], eth_mac[4], eth_mac[5]);
}

static void wifi_prov_print_info(const char *name, const char *pop, const char *transport)
{
    if (!name || !transport)
    {
        ESP_LOGW(TAG, "Cannot generate QR code payload. Data missing.");
        return;
    }
    char payload[150] = {0};
    if (pop)
    {
        snprintf(payload, sizeof(payload), "{\"name\":\"%s\",\"name\":\"%s\""
                                           ",\"pop\":\"%s\",\"transport\":\"%s\"}",
                 name,RD_PASS_WIFI_SOFTAP, pop, transport);
    }
    else
    {
        snprintf(payload, sizeof(payload), "{\"name\":\"%s\""
                                           ",\"transport\":\"%s\"}",
                 name, transport);
    }
    ESP_LOGI(TAG, "If QR code is not visible, data=%s", payload);
}

esp_err_t custom_prov_data_handler(uint32_t session_id, const uint8_t *inbuf, ssize_t inlen,
                                   uint8_t **outbuf, ssize_t *outlen, void *priv_data)
{
    if (inbuf)
    {
        ESP_LOGI(TAG, "Received data: %.*s", inlen, (char *)inbuf);
    }
    char response[] = "SUCCESS";
    *outbuf = (uint8_t *)strdup(response);
    if (*outbuf == NULL)
    {
        ESP_LOGE(TAG, "System out of memory");
        return ESP_ERR_NO_MEM;
    }
    *outlen = strlen(response) + 1; /* +1 for NULL terminating byte */

    return ESP_OK;
}

void reset_wifi_prov_mgr(void)
{
    // delete in4 wifi in NVS
    wifi_prov_mgr_reset_provisioning();
}

static void config_wifi_prov(type_transport_e type_prov)
{
    config_wifi.is_provisioning = true;
    wifi_prov_mgr_config_t config={
        //.scheme = NULL,//wifi_prov_scheme_softap,
        .scheme_event_handler = WIFI_PROV_EVENT_HANDLER_NONE
    };
    // wifi_prov_event_handler_t event_handler = WIFI_PROV_EVENT_HANDLER_NONE;

    if (type_prov == TRANSPORT_BLE)
    {
        deinit_ble_task();
        config.scheme = wifi_prov_scheme_ble;
    }
    else
    {
        config.scheme = wifi_prov_scheme_softap;
    }

    ESP_ERROR_CHECK(wifi_prov_mgr_init(config)); // RD_NOTE: register config
    ESP_LOGI(TAG, "Starting provisioning");
    char service_name[15];
    get_device_service_name(service_name, sizeof(service_name));
    wifi_prov_security_t security = WIFI_PROV_SECURITY_1;
    wifi_prov_security1_params_t *sec_params = PROV_SEC1_PWD; // pop
    char *service_key = NULL;
    if (type_prov == TRANSPORT_BLE)
    {
        service_key = NULL;
    }
    else
    {
        service_key = RD_PASS_WIFI_SOFTAP; // password wifi AP
    }
    wifi_prov_mgr_endpoint_create("custom-data");
    ESP_ERROR_CHECK(wifi_prov_mgr_start_provisioning(security, (const void *)sec_params, service_name, service_key));
    /* The handler for the optional endpoint created above.
     * This call must be made after starting the provisioning, and only if the endpoint
     * has already been created above.
     */
    wifi_prov_mgr_endpoint_register("custom-data", custom_prov_data_handler, NULL);
    const char *pop = PROV_SEC1_PWD;
    if (type_prov == TRANSPORT_BLE)
    {
        wifi_prov_print_info(service_name, pop, PROV_TRANSPORT_BLE);
    }
    else
    {
        wifi_prov_print_info(service_name, pop, PROV_TRANSPORT_SOFTAP); // log
    }
}

void check_provisioning_status(void)
{
    bool provisioned = false;
    reset_wifi_prov_mgr();
    ESP_ERROR_CHECK(wifi_prov_mgr_is_provisioned(&provisioned));
    if (!provisioned)
    {
        ESP_LOGE(TAG, "Device is not provisioned");
    }
    else
    {
        ESP_LOGW(TAG, "Device is provisioned");
        wifi_init_sta_mode();

        wifi_config_t sta_cfg = {0};
        ESP_ERROR_CHECK(esp_wifi_get_config(WIFI_IF_STA, &sta_cfg));
        memcpy(ssid_config, sta_cfg.sta.ssid, sizeof(sta_cfg.sta.ssid));
        memcpy(pass_config, sta_cfg.sta.password, sizeof(sta_cfg.sta.password));
    }
}
void stop_wifi_prov_mgr(void)
{
    wifi_prov_mgr_endpoint_unregister("custom-data"); // RD_NOTE: unregister endpoint
    wifi_prov_mgr_stop_provisioning();
}

void start_wifi_prov_mgr(type_transport_e type_prov){
    config_wifi.type_prov = type_prov;
    config_wifi.time_out = esp_timer_get_time();
    if(config_wifi.is_provisioning){
        stop_wifi_prov_mgr();
        config_wifi.is_config_wifi = true;
    }else{
        config_wifi_prov(config_wifi.type_prov);
    }
    if(TaskCheckTimeOut == NULL){
        xTaskCreate(time_out_prov_task, "time_out_prov_task", 2048, NULL, configMAX_PRIORITIES - 5, &TaskCheckTimeOut);
    }   
}

static void time_out_prov_task(void *param){
    ESP_LOGI("WIFI PROV", "init task check time out");
    while (1)
    {
        if(esp_timer_get_time() - config_wifi.time_out >= TIMEOUT_NETWORK_CONFIGURATION){
            ESP_LOGE("WIFI PROV", "STOP CONFIG WIFI, deinit task check time out");

            if(config_wifi.is_provisioning){
                stop_wifi_prov_mgr();
            }
            vTaskDelay(pdMS_TO_TICKS(100));
            ESP_ERROR_CHECK(esp_wifi_stop());
            vTaskDelay(pdMS_TO_TICKS(100));

            wifi_config_t wifi_cfg = {0};
            memcpy(wifi_cfg.sta.ssid, ssid_config, sizeof(ssid_config));
            memcpy(wifi_cfg.sta.password, pass_config, sizeof(pass_config));

            ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
            ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg));
            ESP_ERROR_CHECK(esp_wifi_start());       
            vTaskDelete(NULL);     
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

static void wifi_connected_task(void *param)
{
    wifi_event_group = xEventGroupCreate();
    init_wifi_prov_mgr();
    check_provisioning_status();
    while (1)
    {
        EventBits_t bits = xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_EVENT, true, true, portMAX_DELAY); // pdMS_TO_TICKS(1000)
        if (bits & WIFI_CONNECTED_EVENT)
        {
            wifi_config_t sta_cfg = {0};
            ESP_ERROR_CHECK(esp_wifi_get_config(WIFI_IF_STA, &sta_cfg));
            ESP_LOGI(TAG, "connected to wifi sta");
            ESP_LOGW(TAG, "ssid: %s", sta_cfg.sta.ssid);
            ESP_LOGW(TAG, "pass: %s", sta_cfg.sta.password);
            memcpy(ssid_config, sta_cfg.sta.ssid, sizeof(sta_cfg.sta.ssid));
            memcpy(pass_config, sta_cfg.sta.password, sizeof(sta_cfg.sta.password));
            // rd_connect_tb();
        }
    }
}


void init_wifi_provision(void){
    xTaskCreate(wifi_connected_task, "wifi_connected_task", 2048 * 2, NULL, configMAX_PRIORITIES - 5, NULL);
}