// #include "esp_err.h"
#include "esp_log.h"
#include "string.h"

#include "device.h"
#include "err_code.h"
#include "rd_common.h"
#include "tb.h"
#include "rd_config.h"

#include "Define.h"

const char *TAG = "DEVICE_MESS";

int get_all_key_value(cJSON *object, cJSON *Json_out){
    if (!cJSON_IsObject(object)) {
        printf("Input is not a JSON object.\n");
        return CODE_EXIT;
    }
    cJSON *item = NULL;
    cJSON_ArrayForEach(item, object){
        const char *key = item->string;
        if (cJSON_IsNumber(item))
        {
            int value = item->valueint;
            cJSON_AddNumberToObject(Json_out, key, value);
        }else if (cJSON_IsString(item))
        {
            char *value = item->valuestring;
            cJSON_AddStringToObject(Json_out, key, value);
        }
        
    }
    return CODE_OK;
}

/*************************************************
*                  RPC callback                  *
*************************************************/
void device_init_rpc_call_back(void)
{
    tb_register_rpc_callback(METHOD_REQUEST_STT_BT, rpc_callback_controlDev);
    tb_register_rpc_callback(METHOD_ADD_DEVICE, rpc_callback_add_home);
    tb_register_rpc_callback(METHOD_DEL_DEVICE, rpc_callback_del_home);
}

int rpc_callback_controlDev(cJSON *reqJson, cJSON *respJson)
{
    ESP_LOGW(TAG, "rpc control device");
    int rsp = do_controlDev_handle(reqJson);
    cJSON *data_param = cJSON_CreateObject();
    cJSON_AddNumberToObject(data_param, "code", rsp);
    get_all_key_value(reqJson, data_param);

    cJSON_AddStringToObject(respJson, "method", METHOD_RESPONE_STT_BT);
    cJSON_AddItemToObject(respJson, "params", data_param);
    return CODE_OK;
}

int rpc_callback_add_home(cJSON *reqJson, cJSON *respJson){
    ESP_LOGW(TAG, "rpc add home");
    int rsp = do_add_home_handle(reqJson);
    cJSON *data_param = cJSON_CreateObject();
    get_all_key_value(reqJson, data_param);
    cJSON_AddNumberToObject(data_param, "code", rsp);
    cJSON_AddNumberToObject(data_param, KEY_TYPE, DEVICE_TYPE);
    cJSON_AddStringToObject(data_param, KEY_NAME, DEVICE_NAME);
    cJSON_AddStringToObject(data_param, KEY_VERSION, DEVICE_VERSION);
    cJSON_AddStringToObject(data_param, KEY_IP, config.ip);
    cJSON_AddStringToObject(data_param, KEY_NAME_WIFI, config.ssid_config);

    cJSON_AddStringToObject(respJson, "method", METHOD_ADD_DEVICE_RSP);
    cJSON_AddItemToObject(respJson, "params", data_param);
    return CODE_OK;
}

int rpc_callback_del_home(cJSON *reqJson, cJSON *respJson){
    ESP_LOGW(TAG, "del home");
    int rsp = do_del_home_handle(reqJson);
    cJSON *data_param = cJSON_CreateObject();
    cJSON_AddNumberToObject(data_param, "code", rsp);
    // get_all_key_value(reqJson, data_param);

    cJSON_AddStringToObject(respJson, "method", METHOD_DEL_DEVICE_RSP);
    cJSON_AddItemToObject(respJson, "params", data_param);
    return CODE_OK;
}

/*************************************************
*                device -> cloud                 *
**************************************************/

int device_publish_telemetry(const char *key, uint8_t value)
{
    int return_value = 0;
    cJSON *data = cJSON_CreateObject();
    cJSON_AddNumberToObject(data, key, value);
    char *dataStr = cJSON_PrintUnformatted(data); // convert JSON -> string
    return_value = tb_publish_telemetry(dataStr);
    free(dataStr);
    cJSON_Delete(data);
    return return_value;
}

int post_stt_btn_server(uint8_t index, uint8_t stt)
{
    int rt_val = 0;
    switch (index)
    {
    case ELE_1:
        rt_val = device_publish_telemetry(KEY_ONOFF_BT1, stt);
        break;
    case ELE_2:
        rt_val = device_publish_telemetry(KEY_ONOFF_BT2, stt);
        break;
    case ELE_3:
        rt_val = device_publish_telemetry(KEY_ONOFF_BT3, stt);
        break;
    case ELE_4:
        rt_val = device_publish_telemetry(KEY_ONOFF_BT4, stt);
        break;
    default:
        rt_val = device_publish_telemetry(KEY_ONOFF_BT1, stt);
        break;
    }
    return rt_val;
}

int device_publish_info(void){
    cJSON *pubJson = cJSON_CreateObject();
    cJSON_AddStringToObject(pubJson, KEY_MAC, config.mac);
    cJSON_AddNumberToObject(pubJson, KEY_TYPE, DEVICE_TYPE);
    cJSON_AddStringToObject(pubJson, KEY_NAME, DEVICE_NAME);
    cJSON_AddStringToObject(pubJson, KEY_VERSION, DEVICE_VERSION);
    cJSON_AddStringToObject(pubJson, KEY_IP, config.ip);
    cJSON_AddStringToObject(pubJson, KEY_NAME_WIFI, config.ssid_config);
    char *dataStr = cJSON_PrintUnformatted(pubJson); // convert JSON -> string
    int return_code = tb_publish_attributes(dataStr);
    free(dataStr);
    cJSON_Delete(pubJson);
    return return_code;
}

int device_rsp_reset_homId(void){
    cJSON *pubJson = cJSON_CreateObject();
    cJSON *param = cJSON_CreateObject();
    cJSON_AddStringToObject(param, KEY_MAC, config.mac);
    cJSON_AddStringToObject(param, KEY_HOME_ID, config.home_id);
    cJSON_AddStringToObject(pubJson, "method", METHOD_RESPONE_RESET_DEVICE);
    cJSON_AddItemToObject(pubJson, "params", param);
    char *dataStr = cJSON_PrintUnformatted(pubJson); // convert JSON -> string
    int return_code = tb_publish_rpc(dataStr);
    free(dataStr);
    cJSON_Delete(pubJson);
    return return_code;
}

/*************************************************
*                Attribute respons               *
*************************************************/
void handle_attributes_message_param_rd(cJSON *attributes_json)
{
	if (cJSON_HasObjectItem(attributes_json, KEY_COUNTDOWN))
	{
		cJSON *countdown_json = cJSON_GetObjectItem(attributes_json, KEY_COUNTDOWN);
		if (cJSON_IsNumber(countdown_json))
		{
			ESP_LOGI("THINGSBOARD", "COUNTDOWN: %d second", countdown_json->valueint);
            rd_set_countdown(0xff, countdown_json->valueint, ON);
		}
	}

    if (cJSON_HasObjectItem(attributes_json, KEY_STATUS_STARTUP))
	{
		cJSON *stt_startup_json = cJSON_GetObjectItem(attributes_json, KEY_STATUS_STARTUP);
		if (cJSON_IsNumber(stt_startup_json))
		{
			ESP_LOGI("THINGSBOARD", "STATUS STARTUP: %d", stt_startup_json->valueint);
            rd_set_status_startup(stt_startup_json->valueint);
		}
	}
}