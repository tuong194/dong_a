#ifndef OTA_H__
#define OTA_H__
#ifdef __cplusplus
extern "C"
{
#endif

	typedef enum
	{
		ESP_OTA_OK = 0,
		ESP_OTA_ERROR,
		ESP_OTA_DATA_ERROR,
		ESP_OTA_DONE,
	} esp_ota_t;

	bool ota_need_update(char *fw_version, int fw_size);
	int ota_write_data(char *ota_write_data, int data_read);
	char *get_esp_version();

	void ota_url(const char *url);
#ifdef __cplusplus
}
#endif
#endif