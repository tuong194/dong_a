#ifndef RD_WIFI_PROV_MGR_H__
#define RD_WIFI_PROV_MGR_H__

#include "stdbool.h"

#define PROV_TRANSPORT_SOFTAP       "softap"
#define PROV_TRANSPORT_BLE          "ble"

#define PROV_SEC1_PWD               "123456"
#define RD_HEADER_SSID_WIFI_SOFTAP  "SW_"       
#define RD_PASS_WIFI_SOFTAP         "RDSMART@2804"
#define RD_RESET_PROV_MGR_ON_FAILURE 1
#define PROV_MGR_MAX_RETRY_CNT  3

// #define RD_PROV_TRANSPORT_BLE        1
// #define RD_PROV_TRANSPORT_SOFTAP     0
// #define RD_CONFIG_PROV_TRANSPORT     RD_PROV_TRANSPORT_BLE
#define TIMEOUT_NETWORK_CONFIGURATION   3*60*1000*1000

typedef enum{
    TRANSPORT_BLE,
    TRANSPORT_SOFTAP
}type_transport_e;

typedef struct{
    type_transport_e type_prov;
    int64_t time_out;
    bool is_config_wifi;
    bool is_provisioning;
}config_wifi_t;

void reset_wifi_prov_mgr(void);
void start_wifi_prov_mgr(type_transport_e type_prov);
void stop_wifi_prov_mgr(void);
void init_wifi_provision(void);

#endif /*  */
