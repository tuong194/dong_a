#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "esp_system.h"
#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "tb.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ota.h"
#include "util.h"
#include "err_code.h"
#include "LC8823.h"
#include "device.h"

#define TAG "Tb"

#define MAX_STRING 128

extern void relay_set(int index, bool value);
extern bool relay_get(int index);
// extern char *get_mac_str();

int startscan_with_app = 0;
extern int nums_new_dev_scan;
extern uint8_t new_dev_scan[50];

static int msg_id = 1;

STAILQ_HEAD(rpc_register_list, rpc_register);
struct rpc_register_list on_device_rpc_callback_list;

// static char *rpc_remote_shell_response = NULL;

static void on_attributes_resp_message(char *topic, char *data, int data_len);
static void on_attributes_message(char *topic, char *data, int data_len);
static void on_rpc_message(char *topic, char *data, int data_len);
static void on_fw_resp_message(char *topic, char *data, int data_len);

// TODO: check topic to call callback
static sub_topic_t sub_topic_list[] = {
		{"v1/devices/me/attributes/response/+", on_attributes_resp_message, 0, 0},
		{"v1/devices/me/attributes", on_attributes_message, 0, 0},
		{"v1/devices/me/rpc/request/+", on_rpc_message, 0, 0},
		{"v2/fw/response/+/chunk/+", on_fw_resp_message, 0, 0}};

#define NUMBER_TOPIC (sizeof(sub_topic_list) / sizeof(sub_topic_t))

tb_status_t tb_publish_string(char *topic, char *data)
{
	ESP_LOGI(TAG, "tb_publish_string topic: %s, data: %s", topic, data);
	tb_status_t rs = mqtt_publish(NULL, topic, data, strlen(data));
	if (rs != CODE_OK)
	{
		ESP_LOGW(TAG, "Tb publish err!");
	}
	return rs;
}

tb_status_t tb_publish_attributes_gateway(char *data)
{
	return tb_publish_string("v1/gateway/attributes", data);
}

tb_status_t tb_publish_dev_add_gate(char *type, char *name)
{
	tb_status_t err = CODE_ERR;
	cJSON *root = cJSON_CreateObject();
	if (root)
	{
		if (cJSON_AddStringToObject(root, "device", name))
		{
			if (cJSON_AddStringToObject(root, "type", type))
			{
				char *string = cJSON_PrintUnformatted(root);
				if (string)
				{
					tb_publish_string("v1/gateway/connect", string);
					free(string);
					err = CODE_OK;
				}
			}
		}
		cJSON_Delete(root);
	}
	return err;
}

tb_status_t tb_publish_dev_offline(uint8_t id)
{
	char data[50];
	sprintf(data, "{\"device\":\"Dev%u\"}", id);
	return tb_publish_string("v1/gateway/disconnect", data);
}

tb_status_t tb_publish_attributes(char *data)
{
	return tb_publish_string("v1/devices/me/attributes", data);
}
tb_status_t tb_publish_macgateway()
{
	char buff[64];
	char mac[32];
	get_mac_str(mac);
	sprintf(buff, "{\"mac\":\"%s\"}", mac);
	return tb_publish_attributes(buff);
}

tb_status_t publish_attributes_request(uint16_t id, char *data)
{
	char topic_req[128];
	sprintf(topic_req, "v1/devices/me/attributes/request/%d", id);
	return tb_publish_string(topic_req, data);
}

tb_status_t tb_publish_telemetry(char *data)
{
	return tb_publish_string("v1/devices/me/telemetry", data);
}

tb_status_t tb_publish_rpc(char *data)
{
	static int i = 0;
	char topic[100];
	sprintf(topic,"%s%d", "v1/devices/me/rpc/request/", ++i);
	return tb_publish_string(topic, data);
}

tb_status_t publish_firmware_request(int msg_id, int current_chunk)
{
	ESP_LOGD(TAG, "publish_firmware_request");
	char topic_req[128];
	sprintf(topic_req, "v2/fw/request/%d/chunk/%d", msg_id, current_chunk);
	return tb_publish_string(topic_req, "5120");
}

static void tb_check_fw_version()
{
	ESP_LOGI(TAG, "tb_check_fw_version");
	publish_attributes_request(++msg_id, "{\"sharedKeys\":\"fw_size,fw_version,device,rule,countdown,statusStartup\"}"); //RD_NOTE: add get status_startup, countdown
}


static int current_chunk = 0;
static int ota_msg_index = 0;
static bool is_updating_fw = false;
static void tb_start_update_firmware()
{
	ESP_LOGI(TAG, "tb_start_update_firmware");
	publish_firmware_request(ota_msg_index, current_chunk);
}

static void handle_attributes_message(cJSON *attributes_json)
{
	if (cJSON_HasObjectItem(attributes_json, "fw_version") &&
			cJSON_HasObjectItem(attributes_json, "fw_size"))
	{
		cJSON *fw_version_json = cJSON_GetObjectItem(attributes_json, "fw_version");
		cJSON *fw_size_json = cJSON_GetObjectItem(attributes_json, "fw_size");
		if (cJSON_IsString(fw_version_json) &&
				cJSON_IsNumber(fw_size_json))
		{
			ESP_LOGI(TAG, "fw_version: %s", fw_version_json->valuestring);
			ESP_LOGI(TAG, "fw_size: %d", fw_size_json->valueint);
			if (is_updating_fw || ota_need_update(fw_version_json->valuestring, fw_size_json->valueint))
			{
				if (!is_updating_fw)
				{
					ESP_LOGI(TAG, "Start update firmware");
					tb_publish_telemetry("{\"fw_state\":\"DOWNLOADING\"}");
					is_updating_fw = true;
					ota_msg_index = ++msg_id;
				}
				tb_start_update_firmware();
				// RD_update_fw(UPDATING_FW_OTA);
			}
		}
	}
	// cJSON *deviceJson = cJSON_GetObjectItem(attributes_json, "device");
	// if (deviceJson)
	// {
	// 	device_manager_remove_all();
	// 	device_manager_add_list_modbus(deviceJson);
	// 	device_manager_print();
	// 	char *modbus_str = cJSON_PrintUnformatted(deviceJson);
	// 	if (modbus_str)
	// 	{
	// 		device_db_write(modbus_str);
	// 		free(modbus_str);
	// 	}
	// }
	// cJSON *ruleJson = cJSON_GetObjectItem(attributes_json, "rule");
	// if (ruleJson)
	// {
	// 	rule_manager_remove_all();
	// 	rule_manager_add_list_rule(ruleJson);
	// 	rule_manager_print();
	// 	char *rule_str = cJSON_PrintUnformatted(ruleJson);
	// 	if (rule_str)
	// 	{
	// 		// rule_db_write(rule_str);
	// 		free(rule_str);
	// 	}
	// }
}

static void on_attributes_resp_message(char *topic, char *data, int data_len)
{
	ESP_LOGI(TAG, "topic: %s", topic);
	ESP_LOGI(TAG, "data=%.*s", data_len, data);
	cJSON *json = cJSON_ParseWithLength(data, data_len);
	if (json)
	{
		if (cJSON_HasObjectItem(json, "shared"))
		{
			cJSON *attributes_json = cJSON_GetObjectItem(json, "shared");
			handle_attributes_message(attributes_json);
			handle_attributes_message_param_rd(attributes_json);
		}
		cJSON_Delete(json);
	}
}

static void on_attributes_message(char *topic, char *data, int data_len)
{
	ESP_LOGI(TAG, "topic: %s", topic);
	ESP_LOGI(TAG, "data=%.*s", data_len, data);
	cJSON *attributes_json = cJSON_ParseWithLength(data, data_len);
	if (attributes_json)
	{
		handle_attributes_message(attributes_json);
		handle_attributes_message_param_rd(attributes_json);
		cJSON_Delete(attributes_json);
	}
}

// static void create_rpc_remote_shell_response(bool done, char *_stdout, char *_stderr)
// {
// 	cJSON *rpc_resp = cJSON_CreateObject();
// 	if (rpc_resp)
// 	{
// 		cJSON *data = cJSON_AddArrayToObject(rpc_resp, "data");
// 		if (data)
// 		{
// 			cJSON *std = cJSON_CreateObject();
// 			if (std)
// 			{
// 				if (_stdout)
// 					cJSON_AddStringToObject(std, "stdout", _stdout);
// 				if (_stderr)
// 					cJSON_AddStringToObject(std, "stderr", _stderr);
// 				cJSON_AddItemToArray(data, std);
// 			}
// 		}
// 		cJSON_AddBoolToObject(rpc_resp, "done", done);
// 		rpc_remote_shell_response = cJSON_PrintUnformatted(rpc_resp);
// 		cJSON_Delete(rpc_resp);
// 	}
// }

static void on_rpc_message(char *topic, char *data, int data_len)
{
	ESP_LOGI(TAG, "topic: %s", topic);
	ESP_LOGI(TAG, "data=%.*s", data_len, data);
	char data_rsp[500] = {0};
	strncpy(data_rsp, data, data_len);
	// response
	char *tb_msg_id = topic + 26; //substr(strlen("v1/devices/me/rpc/request/")
	char topic_rsp[128];
	sprintf(topic_rsp, "v1/devices/me/rpc/response/%s", tb_msg_id);

	cJSON *json = cJSON_ParseWithLength(data, data_len);
	if (json)
	{
		if (cJSON_HasObjectItem(json, "method") && cJSON_HasObjectItem(json, "params"))
		{
			cJSON *method = cJSON_GetObjectItem(json, "method");
			cJSON *params = cJSON_GetObjectItem(json, "params");
			if (cJSON_IsString(method))
			{
				char *cmd = method->valuestring;
				rpc_register_t *rpc_register = NULL;
				STAILQ_FOREACH(rpc_register, &on_device_rpc_callback_list, entries)
				{
					if (strcmp(cmd, rpc_register->cmd) == 0)
					{
						cJSON *respJson = cJSON_CreateObject();
						if (respJson)
						{
							int rs = rpc_register->func(params, respJson); //call back func RPC
							if (rs == CODE_OK)
							{
								ESP_LOGI(TAG, "Call %s OK, rs: %d", cmd, rs);
								char *respStr = cJSON_PrintUnformatted(respJson); //convert JSON -> string
								if (respStr)
								{
									tb_publish_string(topic_rsp, respStr);
									free(respStr);
								}
							}
							else
							{
								ESP_LOGW(TAG, "Call %s Error, rs: %d", cmd, rs);
							}
							cJSON_Delete(respJson);
						}
						break;
					}
				}
			}
		}
		cJSON_Delete(json);
		return;
	}
}

static void on_fw_resp_message(char *topic, char *data, int data_len)
{
	static int binary_size = 0;
	binary_size += data_len;
	ESP_LOGI(TAG, "binary_size: %d", binary_size);
	// TODO: need check binary_size and timeout if server not response, reset current_chunk=0 when done
	char topic_rsp[128];
	sprintf(topic_rsp, "v2/fw/response/%d/chunk/%d", ota_msg_index, current_chunk);
	if (strcmp(topic_rsp, topic) == 0)
	{
		int rs = ota_write_data(data, data_len);
		if (rs == ESP_OTA_DONE)
		{
			tb_publish_telemetry("{\"fw_state\":\"UPDATED\"}");
			ESP_LOGI(TAG, "Prepare to restart system!");
			// RD_update_fw(UPDATE_OTA_SUCCESS);
			vTaskDelay(6000 / portTICK_PERIOD_MS);
			esp_restart();
		}
		else if (rs == ESP_OTA_OK)
		{
			publish_firmware_request(ota_msg_index, ++current_chunk);
		}
		else if (rs == ESP_OTA_DATA_ERROR)
		{
			current_chunk = 0;
			publish_firmware_request(ota_msg_index, current_chunk);
		}
		else
		{
			ESP_LOGI(TAG, "Prepare to restart system!");
			esp_restart();
		}
	}
}

static void on_connected(int result)
{
	if (result == 0)
	{
		ESP_LOGI(TAG, "connected");
		msg_id = 0;
		config_stt_connect_mqtt(1); //RD_NOTE: led wifi
		device_publish_info();
		// tb_publish_macgateway();
		if (NUMBER_TOPIC > 0)
			mqtt_subscribe(&sub_topic_list[0].msg_id, sub_topic_list[0].topic);
		// TODO: check result
	}
	else
	{
		ESP_LOGW(TAG, "connect faild, reconnecting...");
		// sleep(1);
		// if (mqtt_driver.connect)
		// 	mqtt_driver.connect();
	}
}

static void on_disconnected(int result)
{
	ESP_LOGW(TAG, "disconnected, reconnecting...");
	config_stt_connect_mqtt(0);
	// for (int i = 0; i < NUMBER_TOPIC; i++)
	// {
	// 	sub_topic_list[i].state = false;
	// }
	// sleep(1);
	// if (mqtt_driver.connect)
	// 	mqtt_driver.connect();
}

static void on_subscribed(int msg_id, int result)  //RD_NOTE sub all topic
{
	ESP_LOGI(TAG, "subscribed msg_id: %d", msg_id);
	for (int i = 0; i < NUMBER_TOPIC; i++)
	{
		if (sub_topic_list[i].msg_id == msg_id)
			sub_topic_list[i].state = true;
	}
	for (int i = 0; i < NUMBER_TOPIC; i++)
	{
		if (!sub_topic_list[i].state)
		{
			mqtt_subscribe(&sub_topic_list[i].msg_id, sub_topic_list[i].topic);
			// TODO: check result
			return;
		}
	}
	tb_check_fw_version(); // RD_NOTE: check firm version and update if needed
}

static void on_unsubscribed(int msg_id, int result)
{
	ESP_LOGI(TAG, "unsubscribed");
}

static void on_published(int msg_id, int result)
{
	ESP_LOGI(TAG, "published");
}

// util function
static bool check_mqtt_topic(char *recieved_topic, char *sub_topic)
{
	int len_a_word = 0;
	int j = 0;
	int sub_topic_len = strlen(sub_topic);
	for (int i = 0; i < strlen(recieved_topic); i++)
	{
		if (sub_topic[j] == '/' && recieved_topic[i] == '/')
		{
			len_a_word = 0;
		}
		else if (sub_topic[j] == '#')
		{
			if (len_a_word) // co ki tu dung truoc # nen khong hop le
				return false;
			if (j + 1 == sub_topic_len) // ky tu cuoi cung la # nen OK het
				return true;
			return false; // co ki tu dung sau # nen khong hop le
		}
		else if (sub_topic[j] == '+')
		{
			if (len_a_word) // co ki tu dung truoc + nen khong hop le
				return false;
			if (sub_topic[j + 1] == '\0' || sub_topic[j + 1] == '/') // ky tu + dung 1 minh
			{
				for (int k = i; k <= strlen(recieved_topic); k++) // bo qua 1 buoc voi recieved_topic
				{
					if (recieved_topic[k] == '\0' || recieved_topic[k] == '/')
					{
						i = k - 1;
						break;
					}
				}
			}
		}
		else if (sub_topic[j] != recieved_topic[i])
		{
			return false;
		}
		else
		{
			len_a_word++;
		}
		if (j < sub_topic_len)
			j++;
		else
			return false;
	}
	if (j != sub_topic_len)
		return false;
	return true;
}

static void on_data(int msg_id, char *topic, char *payload, int payload_len)
{
	ESP_LOGD(TAG, "data");
	// ESP_LOGD(TAG, "TOPIC=%s", topic);
	// ESP_LOGD(TAG, "DATA=%.*s", payload_len, payload);
	for (int i = 0; i < NUMBER_TOPIC; i++)
	{
		if (check_mqtt_topic(topic, sub_topic_list[i].topic))
		{
			if (sub_topic_list[i].handler)
				sub_topic_list[i].handler(topic, payload, payload_len);
		}
	}
}

static mqtt_callback_driver_t tb_callback_driver = {
		.on_connected = on_connected,
		.on_disconnected = on_disconnected,
		.on_subscribed = on_subscribed,
		.on_unsubscribed = on_unsubscribed,
		.on_published = on_published,
		.on_data = on_data,
};

void tb_init()
{
	ESP_LOGI(TAG, "tb_init");

	STAILQ_INIT(&on_device_rpc_callback_list);

	mqtt_register_callback_driver(&tb_callback_driver);
}

int tb_connect(const char *username, const char *password)
{
	return mqtt_connect(username, password);
}

int tb_mqtt_disconnect()
{
	return mqtt_disconnect();
}

bool tb_is_connected()
{
	return mqtt_is_connected();
}

void tb_register_rpc_callback(const char *cmd, rpc_callback_func rpc_callback)
{
	rpc_register_t *rpc_register = NULL;
	STAILQ_FOREACH(rpc_register, &on_device_rpc_callback_list, entries)
	{
		if (strcmp(cmd, rpc_register->cmd) == 0)
		{
			rpc_register->func = rpc_callback;
			return;
		}
	}

	rpc_register = (rpc_register_t *)malloc(sizeof(rpc_register_t));
	if (rpc_register)
	{
		strcpy(rpc_register->cmd, cmd);
		rpc_register->func = rpc_callback;
		STAILQ_INSERT_TAIL(&on_device_rpc_callback_list, rpc_register, entries);
	}
}

