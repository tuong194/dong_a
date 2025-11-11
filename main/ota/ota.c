#include <string.h>
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_app_format.h"
#include "esp_flash_partitions.h"
#include "esp_partition.h"
#include "ota.h"

#define HASH_LEN 32
#define TAG "Ota"

static bool image_header_was_checked = false;
static esp_ota_handle_t update_handle = 0;
static int binary_file_length = 0;
static const esp_partition_t *update_partition = NULL;
static int file_fw_size = 0;
char fw_version_esp[20];

bool ota_need_update(char *fw_version, int fw_size)
{
	ESP_LOGW(TAG, "ota_need_update");
	esp_app_desc_t running_app_info;
	const esp_partition_t *running = esp_ota_get_running_partition();
	if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK)
	{
		ESP_LOGW(TAG, "update version: %s, running version: %s", fw_version, running_app_info.version);
		if (strcmp(fw_version, running_app_info.version)) // khac firm version
		{
			image_header_was_checked = false;
			binary_file_length = 0;
			file_fw_size = fw_size;
			return true;
		}
	}
	return false;
}

char *get_esp_version()
{
	const esp_partition_t *running = esp_ota_get_running_partition();
	esp_app_desc_t running_app_info;
	if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK)
	{
		if (strlen(running_app_info.version) < 18)
		{
			strcpy(fw_version_esp, running_app_info.version);
			return fw_version_esp;
		}
	}
	return NULL;
}

int ota_write_data(char *ota_write_data, int data_read)
{
	ESP_LOGI(TAG, "ota_write_data");
	esp_err_t err;
	if (image_header_was_checked == false)
	{
		esp_app_desc_t new_app_info;
		if (data_read > sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t) + sizeof(esp_app_desc_t))
		{
			// check current version with downloading
			memcpy(&new_app_info, &ota_write_data[sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t)], sizeof(esp_app_desc_t));
			ESP_LOGI(TAG, "New firmware version: %s", new_app_info.version);

			esp_app_desc_t running_app_info;
			const esp_partition_t *running = esp_ota_get_running_partition();
			if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK)
			{
				ESP_LOGI(TAG, "Running firmware version: %s", running_app_info.version);
			}

			update_partition = esp_ota_get_next_update_partition(NULL);
			assert(update_partition != NULL);
			ESP_LOGI(TAG, "Writing to partition subtype %d at offset 0x%" PRIx32,
							 update_partition->subtype, update_partition->address);

			image_header_was_checked = true;

			err = esp_ota_begin(update_partition, OTA_WITH_SEQUENTIAL_WRITES, &update_handle);
			if (err != ESP_OK)
			{
				ESP_LOGE(TAG, "esp_ota_begin failed (%s)", esp_err_to_name(err));
				esp_ota_abort(update_handle);
				// task_fatal_error();
				return ESP_OTA_ERROR;
			}
			ESP_LOGI(TAG, "esp_ota_begin succeeded");
		}
		else
		{
			ESP_LOGE(TAG, "received package is not fit len");
			// esp_ota_abort(update_handle);
			// task_fatal_error();
			image_header_was_checked = false;
			binary_file_length = 0;
			return ESP_OTA_DATA_ERROR;
		}
	}
	err = esp_ota_write(update_handle, (const void *)ota_write_data, data_read);
	if (err != ESP_OK)
	{
		esp_ota_abort(update_handle);
		// task_fatal_error();
		return ESP_OTA_ERROR;
	}
	binary_file_length += data_read;
	ESP_LOGD(TAG, "Written image length %d", binary_file_length);
	if (file_fw_size == binary_file_length)
	{
		err = esp_ota_end(update_handle);
		if (err != ESP_OK)
		{
			if (err == ESP_ERR_OTA_VALIDATE_FAILED)
			{
				ESP_LOGE(TAG, "Image validation failed, image is corrupted");
				image_header_was_checked = false;
				binary_file_length = 0;
				return ESP_OTA_DATA_ERROR;
			}
			else
			{
				ESP_LOGE(TAG, "esp_ota_end failed (%s)!", esp_err_to_name(err));
			}
			// task_fatal_error();
			return ESP_OTA_ERROR;
		}

		err = esp_ota_set_boot_partition(update_partition);
		if (err != ESP_OK)
		{
			ESP_LOGE(TAG, "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
			// task_fatal_error();
			return ESP_OTA_ERROR;
		}
		return ESP_OTA_DONE;
	}
	return ESP_OTA_OK;
}

static void ota_url_task(void *pvParameter)
{
	char *url_m = (char *)pvParameter;
	if (url_m)
	{
		ESP_LOGI(TAG, "Starting OTA url: %s", url_m);
		esp_http_client_config_t config = {
				.url = url_m,
				.keep_alive_enable = true,
		};
		esp_https_ota_config_t ota_config = {
				.http_config = &config,
		};
		ESP_LOGI(TAG, "Attempting to download update from %s", config.url);
		esp_err_t ret = esp_https_ota(&ota_config);
		if (ret == ESP_OK)
		{
			ESP_LOGI(TAG, "OTA Succeed, Rebooting...");
			esp_restart();
		}
		else
		{
			ESP_LOGE(TAG, "Firmware upgrade failed");
		}
		free(url_m);
	}
	else
	{
		ESP_LOGW(TAG, "Firmware upgrade failed, url null");
	}
	vTaskDelete(NULL);
}

void ota_url(const char *url)
{
	char *url_m = malloc(strlen(url) + 1);
	strcpy(url_m, url);
	xTaskCreate(&ota_url_task, "ota_url_task", 8192, url_m, 5, NULL);
}