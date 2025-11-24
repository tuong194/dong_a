#include "device.h"
#include "string.h"
#include "err_code.h"
#include "esp_log.h"
#include "rd_common.h"
#include "LC8823.h"
#include "rd_config.h"
#include "tb.h"

#include "Define.h"

const char *TAG_DEVICE = "DEVICE";

key_map_t key_map[] =
{
    {KEY_COUNTDOWN, COUNTDOWN},
    {KEY_ONOFF, ONOFF},
    {KEY_STATUS_STARTUP, STATUS_STARTUP},

    {KEY_ONOFF_BT1, ONOFF_BT1},
    {KEY_ONOFF_BT2, ONOFF_BT2},
    {KEY_ONOFF_BT3, ONOFF_BT3},
    {KEY_ONOFF_BT4, ONOFF_BT4},

    {KEY_RED_ON_1, RED_ON_1},
    {KEY_BLUE_ON_1, BLUE_ON_1},
    {KEY_GREEN_ON_1, GREEN_ON_1},
    {KEY_DIM_ON_1, DIM_ON_1},

    {KEY_RED_OFF_1, RED_OFF_1},
    {KEY_BLUE_OFF_1, BLUE_OFF_1},
    {KEY_GREEN_OFF_1, GREEN_OFF_1},
    {KEY_DIM_OFF_1, DIM_OFF_1},

    {KEY_RED_ON_2, RED_ON_2},
    {KEY_BLUE_ON_2, BLUE_ON_2},
    {KEY_GREEN_ON_2, GREEN_ON_2},
    {KEY_DIM_ON_2, DIM_ON_2},

    {KEY_RED_OFF_2, RED_OFF_2},
    {KEY_BLUE_OFF_2, BLUE_OFF_2},
    {KEY_GREEN_OFF_2, GREEN_OFF_2},
    {KEY_DIM_OFF_2, DIM_OFF_2},

    {KEY_RED_ON_3, RED_ON_3},
    {KEY_BLUE_ON_3, BLUE_ON_3},
    {KEY_GREEN_ON_3, GREEN_ON_3},
    {KEY_DIM_ON_3, DIM_ON_3},

    {KEY_RED_OFF_3, RED_OFF_3},
    {KEY_BLUE_OFF_3, BLUE_OFF_3},
    {KEY_GREEN_OFF_3, GREEN_OFF_3},
    {KEY_DIM_OFF_3, DIM_OFF_3},

    {KEY_RED_ON_4, RED_ON_4},
    {KEY_BLUE_ON_4, BLUE_ON_4},
    {KEY_GREEN_ON_4, GREEN_ON_4},
    {KEY_DIM_ON_4, DIM_ON_4},

    {KEY_RED_OFF_4, RED_OFF_4},
    {KEY_BLUE_OFF_4, BLUE_OFF_4},
    {KEY_GREEN_OFF_4, GREEN_OFF_4},
    {KEY_DIM_OFF_4, DIM_OFF_4}
};

keyID get_key_id(const char *key)
{
    for (size_t i = 0; i < UNKNOWN_KEY; i++)
    {
        if (!strcmp(key, key_map[i].key))
        {
            return key_map[i].ID;
        }
    }
    return UNKNOWN_KEY;
}

int get_value(cJSON *reqJson, const char *key)
{
    cJSON *value_json = cJSON_GetObjectItem(reqJson, key);
    int value = 0;
    if (cJSON_IsNumber(value_json))
    {
        value = value_json->valueint;
    }
    return value;
}


static uint8_t count_key_color = 0;
static uint8_t count_key_dim = 0;

static void set_color(uint8_t index, uint8_t onoff, rgb_t rgb, uint8_t val){
    static uint8_t blue, green, red;
    switch (rgb)
    {
    case RED:
        red = val;
        count_key_color++;
        break;
    case GREEN:
        green = val;
        count_key_color++;
        break;
    case BLUE:
        blue = val;
        count_key_color++;
        break;
    default:
        break;
    }
    // ESP_LOGI("DEVICE", "count key COLOR: %d", count_key_color);
    if(count_key_color == 3){
        // ESP_LOGE("DEVICE", "set color ok");
        lc8823_set_color(index, onoff, red, green, blue);
        count_key_color = 0;
    }
}

static void set_dim_onoff(uint8_t index, uint8_t onoff, uint8_t dim_value){
    static uint8_t dim_on, dim_off;
    if(onoff == ON_STT){
        dim_on = dim_value;
        count_key_dim++;
    }else if(onoff == OFF_STT){
        dim_off = dim_value;
        count_key_dim++;
    }
    // ESP_LOGE("DEVICE", "count key DIM: %d", count_key_dim);
    if(count_key_dim == 2){
        ESP_LOGI("DEVICE", "set DIM ok");
        lc8823_set_dim_onoff(index, ON_STT, dim_on);
        lc8823_set_dim_onoff(index, OFF_STT, dim_off);
        count_key_dim = 0;
    }
}

static int set_onoff_all(uint8_t onoff){
    for (uint8_t i = 0; i < NUM_ELEMENT; i++)
    {
        control_set_onoff(i, onoff);
    }
    return CODE_OK;
}

int do_controlDev_handle(cJSON *reqJson)
{
    int rsp_stt = CODE_OK;
    cJSON *item = NULL;
    keyID key_id = 0;
    cJSON_ArrayForEach(item, reqJson)
    {
        key_id = get_key_id(item->string);
        //int value_req = get_value(reqJson, item->string);
        int value_req = 0;//= item->valueint;
        if (cJSON_IsNumber(item))
        {
            value_req = item->valueint;
        }
        printf("KEY: %s, VAlue: %d\n", item->string, value_req);

        switch (key_id)
        {
        case ONOFF:
            set_onoff_all(value_req);
            break;

        case ONOFF_BT1:
            control_set_onoff(ELE_1, value_req);
            break;
        case ONOFF_BT2:
            control_set_onoff(ELE_2, value_req);
            break;
        case ONOFF_BT3:
            control_set_onoff(ELE_3, value_req);
            break;
        case ONOFF_BT4:
            control_set_onoff(ELE_4, value_req);
            break;

        case RED_ON_1:
            set_color(ELE_1, ON_STT, RED, value_req);
            break;
        case BLUE_ON_1:
            set_color(ELE_1, ON_STT, BLUE, value_req);
            break;
        case GREEN_ON_1:
            set_color(ELE_1, ON_STT, GREEN, value_req);
            break;
        case DIM_ON_1:
            set_dim_onoff(ELE_1, ON_STT, value_req);
            break;
        case RED_OFF_1:
            set_color(ELE_1, OFF_STT, RED, value_req);
            break;
        case BLUE_OFF_1:
            set_color(ELE_1, OFF_STT, BLUE, value_req);
            break;
        case GREEN_OFF_1:
            set_color(ELE_1, OFF_STT, GREEN, value_req);
            break;
        case DIM_OFF_1:
            set_dim_onoff(ELE_1, OFF_STT, value_req);
            break;

        case RED_ON_2:
            set_color(ELE_2, ON_STT, RED, value_req);
            break;
        case BLUE_ON_2:
            set_color(ELE_2, ON_STT, BLUE, value_req);
            break;
        case GREEN_ON_2:
            set_color(ELE_2, ON_STT, GREEN, value_req);
            break;
        case DIM_ON_2:
            set_dim_onoff(ELE_2, ON_STT, value_req);
            break;
        case RED_OFF_2:
            set_color(ELE_2, OFF_STT, RED, value_req);
            break;
        case BLUE_OFF_2:
            set_color(ELE_2, OFF_STT, BLUE, value_req);
            break;
        case GREEN_OFF_2:
            set_color(ELE_2, OFF_STT, GREEN, value_req);
            break;
        case DIM_OFF_2:
            set_dim_onoff(ELE_2, OFF_STT, value_req);
            break;

        case RED_ON_3:
            set_color(ELE_3, ON_STT, RED, value_req);
            break;
        case BLUE_ON_3:
            set_color(ELE_3, ON_STT, BLUE, value_req);
            break;
        case GREEN_ON_3:
            set_color(ELE_3, ON_STT, GREEN, value_req);
            break;
        case DIM_ON_3:
            set_dim_onoff(ELE_3, ON_STT, value_req);
            break;
        case RED_OFF_3:
            set_color(ELE_3, OFF_STT, RED, value_req);
            break;
        case BLUE_OFF_3:
            set_color(ELE_3, OFF_STT, BLUE, value_req);
            break;
        case GREEN_OFF_3:
            set_color(ELE_3, OFF_STT, GREEN, value_req);
            break;
        case DIM_OFF_3:
            set_dim_onoff(ELE_3, OFF_STT, value_req);
            break;

        case RED_ON_4:
            set_color(ELE_4, ON_STT, RED, value_req);
            break;
        case BLUE_ON_4:
            set_color(ELE_4, ON_STT, BLUE, value_req);
            break;
        case GREEN_ON_4:
            set_color(ELE_4, ON_STT, GREEN, value_req);
            break;
        case DIM_ON_4:
            set_dim_onoff(ELE_4, ON_STT, value_req);
            break;
        case RED_OFF_4:
            set_color(ELE_4, OFF_STT, RED, value_req);
            break;
        case BLUE_OFF_4:
            set_color(ELE_4, OFF_STT, BLUE, value_req);
            break;
        case GREEN_OFF_4:
            set_color(ELE_4, OFF_STT, GREEN, value_req);
            break;
        case DIM_OFF_4:
            set_dim_onoff(ELE_4, OFF_STT, value_req);
            break;

        default:
            ESP_LOGE("DEVICE", "UNKNOWN KEY method \"controlDev\" ");
            rsp_stt = CODE_FORMAT_ERROR;
            break;
        }
    }
    count_key_dim = 0;
    count_key_color = 0;

    return rsp_stt;
}

int do_add_home_handle(cJSON *reqJson){
    if(cJSON_HasObjectItem(reqJson, KEY_MAC) && cJSON_HasObjectItem(reqJson, KEY_HOME_ID)){
        char *mac = "";
        char *homeId = "";
        cJSON *mac_obj = cJSON_GetObjectItem(reqJson, KEY_MAC);
		cJSON *homeid_obj = cJSON_GetObjectItem(reqJson, KEY_HOME_ID);
        if (cJSON_IsString(mac_obj))
        {
            mac = mac_obj->valuestring;
        }
        if (cJSON_IsString(homeid_obj))
        {
            homeId = homeid_obj->valuestring;
        }
        printf("mac: %s, home_id: %s\n\n", mac, homeId);

        if(strcmp(mac, config.mac) == 0){
            if(check_add_home()){
                ESP_LOGW(TAG_DEVICE, "vao nha roi nha");
                return CODE_NOT_RESPONSE;
            }else{
				printf("CHECK OK\n\n");
				Config_SetHomeId(homeId);
			
				return CODE_OK;
            }
        }else{
            ESP_LOGE(TAG_DEVICE, "check mac fail");
            return CODE_NOT_RESPONSE;
        }
    }    
    return CODE_FORMAT_ERROR;
}

int do_del_home_handle(cJSON *reqJson){
    if(cJSON_HasObjectItem(reqJson, KEY_MAC) && cJSON_HasObjectItem(reqJson, KEY_HOME_ID)){
        char *mac = "";
        char *homeId = "";
        cJSON *mac_obj = cJSON_GetObjectItem(reqJson, KEY_MAC);
		cJSON *homeid_obj = cJSON_GetObjectItem(reqJson, KEY_HOME_ID);
        if (cJSON_IsString(mac_obj))
        {
            mac = mac_obj->valuestring;
        }
        if (cJSON_IsString(homeid_obj))
        {
            homeId = homeid_obj->valuestring;
        }
        printf("mac: %s, home_id: %s\n\n", mac, homeId);

        if(strcmp(mac, config.mac) == 0 && strcmp(homeId, config.home_id) == 0){
            ESP_LOGW(TAG_DEVICE, "xoa nha");
            Config_SetHomeId(HOME_ID_DEFAULT);
			reset_config_device();
            return CODE_OK;
        }else{
            ESP_LOGE(TAG_DEVICE, "check mac or homeID fail");
            return CODE_NOT_RESPONSE;
        }
    } 
    return CODE_FORMAT_ERROR;
}


