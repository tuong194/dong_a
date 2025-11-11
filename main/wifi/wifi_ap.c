#include "esp_wifi.h"
#include "esp_log.h"

#define TAG "WIFI_AP"

void wifi_ap_init()
{
  esp_netif_t *wifi_netif = esp_netif_create_default_wifi_ap();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  /* Register our event handler for Wi-Fi, IP and Provisioning related events */
  // ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
  // ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
  // ESP_ERROR_CHECK(esp_wifi_set_mode(mode));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH));
  ESP_ERROR_CHECK(esp_wifi_start());
  ESP_LOGI(TAG, "Wi-Fi AP started");
}