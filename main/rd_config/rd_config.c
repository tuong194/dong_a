#include <stdio.h>
#include <string.h>
#include "nvs_flash.h"
#include "nvs.h"

#include "rd_config.h"
#include "esp_log.h"
#include "esp_mac.h"

#define NVS_NAME_SPACE "my_nvs"

Config_t config;

/****************************************
 *           Internal Helpers           *
 ****************************************/

static bool set_str_config_entry(const char *key, const char *value) {
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAME_SPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return false;
    }

    err = nvs_set_str(handle, key, value);
    if (err != ESP_OK) {
        nvs_close(handle);
        return false;
    }

    err = nvs_commit(handle);
    nvs_close(handle);
    return err == ESP_OK;
}

static bool set_int_config_entry(const char *key, int value) {
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAME_SPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return false;
    }

    err = nvs_set_i32(handle, key, value);
    if (err != ESP_OK) {
        nvs_close(handle);
        return false;
    }

    err = nvs_commit(handle);
    nvs_close(handle);
    return err == ESP_OK;
}

static bool set_bool_config_entry(const char *key, int value) {
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAME_SPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return false;
    }

    err = nvs_set_u8(handle, key, value);
    if (err != ESP_OK) {
        nvs_close(handle);
        return false;
    }

    err = nvs_commit(handle);
    nvs_close(handle);
    return err == ESP_OK;
}
 
/****************************************
 *                API                   *
 ****************************************/

void Config_Print() {
    ESP_LOGW("Config", "host: %s", config.host);
    ESP_LOGW("Config", "port: %d", config.port);
    ESP_LOGW("Config", "clientId: %s", config.clientId);
    ESP_LOGW("Config", "username: %s", config.username);
    ESP_LOGW("Config", "password: %s", config.password);
    ESP_LOGW("Config", "keepAlive: %d", config.keepAlive);
    ESP_LOGW("Config", "add home ? : %d", check_add_home());
}

static void Get_Mac_Device(void){
    uint8_t mac_esp[6] = {0};
    // esp_efuse_mac_get_default(mac_esp);
    esp_read_mac(mac_esp, ESP_MAC_WIFI_STA);
   
    sprintf(config.mac, "%02x%02x%02x%02x%02x%02x", mac_esp[0], mac_esp[1], mac_esp[2], mac_esp[3], mac_esp[4], mac_esp[5]);
    printf("config mac: %s\n", config.mac);
    sprintf(config.clientId, "rd_%s", config.mac);
    sprintf(config.username, "rd_%s", config.mac);
    sprintf(config.password, "rd_%s", config.mac);
}

void Config_Read_Flash() {
    // printf("\n\n\njkasfhjksadhfjkasdh");
    char str_temp[STRING_VALUE_MAX_SIZE];
    int32_t int_temp;
    bool bool_temp;

    nvs_handle_t handle;
    Get_Mac_Device();
    esp_err_t err = nvs_open(NVS_NAME_SPACE, NVS_READWRITE, &handle);

    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return;
    }

    // Server
    size_t len = sizeof(str_temp);
    if (nvs_get_str(handle, HOST_KEY, str_temp, &len) == ESP_OK)
        strncpy(config.host, str_temp, sizeof(config.host));
    else
        strcpy(config.host, HOST_DEFAULT);

    if (nvs_get_i32(handle, PORT_KEY, &int_temp) == ESP_OK)
        config.port = int_temp;
    else
        config.port = PORT_DEFAULT;

    if (nvs_get_i32(handle, KEEP_ALIVE_KEY, &int_temp) == ESP_OK)
        config.keepAlive = int_temp;
    else
        config.keepAlive = KEEP_ALIVE_DEFAULT;

    if (nvs_get_u8(handle, TLS_KEY, &bool_temp) == ESP_OK)
        config.tls = bool_temp;
    else
        config.tls = TLS_DEFAULT;
    
    if (nvs_get_str(handle, HOME_ID_KEY, str_temp, &len) == ESP_OK)
        strncpy(config.home_id, str_temp, sizeof(config.home_id));
    else
        strcpy(config.home_id, HOME_ID_DEFAULT);

    
    nvs_close(handle);

    Config_Print();
}


/****************************************
 *        Setters (partial example)     *
 ****************************************/

bool Config_SetHost(const char *host) {
    return set_str_config_entry(HOST_KEY, host);
}

bool Config_SetPort(int port) {
    return set_int_config_entry(PORT_KEY, port);
}

bool Config_SetClientId(const char *clientId) {
    return set_str_config_entry(CLIENT_ID_KEY, clientId);
}
bool Config_SetUsername(const char *username) {
    return set_str_config_entry(USERNAME_KEY, username);
}
bool Config_SetPassword(const char *password) {
    return set_str_config_entry(PASSWORD_KEY, password);
}

bool Config_SetKeepAlive(int keepAlive) {
    return set_int_config_entry(KEEP_ALIVE_KEY, keepAlive);
}
bool Config_SetTLS(bool tls) {
    return set_bool_config_entry(TLS_KEY, tls);
}

bool Config_SetHomeId(const char *home_id) {
    strncpy(config.home_id, home_id, sizeof(config.home_id));
    printf("set home id: %s\n", config.home_id);
    return set_str_config_entry(HOME_ID_KEY, home_id);
}

bool check_add_home(void) {
    if(!strcmp(config.home_id, HOME_ID_DEFAULT)){
        return false;
    }
    return true;
}

