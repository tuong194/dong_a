#include "rd_flash.h"

#include "esp_flash.h"
#include "esp_log.h"
#include "esp_err.h"
#include "nvs.h"
#include "nvs_flash.h"

static const char *FLASH_TAG = "FLASH_TAG";
static nvs_handle_t rd_nvs_handle;

/*---------------addr-----------------*/
#if TYPE_FLASH == FLASH_TYPE_ADDR
void rd_flash_read(uint32_t address, void *buffer, uint32_t length){
    ESP_LOGI(FLASH_TAG,"Read flash at 0x%03x  \n", (unsigned int)address);
    ESP_ERROR_CHECK(esp_flash_read(NULL, buffer, address, length));
}

void rd_flash_write(uint32_t address, void *buffer, uint32_t length){
    ESP_LOGI(FLASH_TAG,"Write flash at 0x%03x  \n", (unsigned int)address);
    ESP_ERROR_CHECK(esp_flash_erase_region(NULL, address, 4096));
    ESP_ERROR_CHECK(esp_flash_write(NULL, buffer, address, length));
}

/*---------------key-value-----------------*/
#elif TYPE_FLASH == FLASH_TYPE_KEY_VALUE
void rd_write_flash(const char* key, void *data, size_t length)
{
    esp_err_t err;
    err = nvs_open(NAME_SPACE, NVS_READWRITE, &rd_nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(FLASH_TAG, "Failed to open NVS!\n");
        return;
    }
    //nvs_erase_key(rd_nvs_handle, key); // xoa du lieu trong key
    ESP_ERROR_CHECK(nvs_set_blob(rd_nvs_handle, key, (const void *)data, length));
    nvs_commit(rd_nvs_handle);
    nvs_close(rd_nvs_handle);
    ESP_LOGI("RD_FLASH","write data to key %s\n", key);
}

void rd_read_flash(const char* key, void *data_rec, size_t length)
{
    esp_err_t err;
    err= nvs_open(NAME_SPACE, NVS_READWRITE, &rd_nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(FLASH_TAG, "Failed to open NVS!\n");
        return;
    }
    esp_err_t err2 = nvs_get_blob(rd_nvs_handle, key, (void *)data_rec, &length);
    if (err2 != ESP_OK) {
        ESP_LOGE(FLASH_TAG, "Failed to read data!\n");
        return;
    }
    nvs_close(rd_nvs_handle);
    
} 
void rd_flash_erase(const char* key)
{
    esp_err_t err;
    err= nvs_open(NAME_SPACE, NVS_READWRITE, &rd_nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(FLASH_TAG, "Failed to open NVS!\n");
        return;
    }
    //nvs_erase_all(rd_nvs_handle);
    nvs_erase_key(rd_nvs_handle, key); // xoa du lieu trong key
    nvs_commit(rd_nvs_handle);
    nvs_close(rd_nvs_handle);
    printf("erase data from %s\n", key);
}

size_t get_size(const char* key){
    size_t length;
    esp_err_t err;
    err= nvs_open(NAME_SPACE, NVS_READWRITE, &rd_nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(FLASH_TAG, "Failed to open NVS!\n");
        return 0;
    }
    esp_err_t err2 = nvs_get_blob(rd_nvs_handle, key, NULL, &length);
    if (err2 != ESP_OK) {
        ESP_LOGE(FLASH_TAG, "Failed to read data!\n");
        return 0;
    }
    nvs_close(rd_nvs_handle);
    return length;
}
#endif

void check_nvs_space(void) {
    nvs_stats_t nvs_stats;
    nvs_get_stats(NULL, &nvs_stats);
    printf("NVS Stats: Used entries = %d, Free entries = %d, Total entries = %d\n",
           nvs_stats.used_entries, nvs_stats.free_entries, nvs_stats.total_entries);
}

void rd_flash_init(void)
{
    esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
        printf("init flash\n");  
	}
	ESP_ERROR_CHECK(ret);
    check_nvs_space();
}


