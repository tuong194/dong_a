#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_task_wdt.h"
#include "esp_log.h"
#include "esp_timer.h"

#include "K9B_Remote.h"
#include "rd_flash.h"
#include "rd_common.h"
#include "ble_adv.h"

extern QueueHandle_t adv_queue_ble;

static switchKP9_proxy_t *switchKP9_proxy;
static Sw_Working_Stt_Str Sw_Working_Stt_Val;

flash_K9B_onoff_t flash_K9B_onoff;

static void init_flash_K9B_onoff_default(void)
{
	flash_K9B_onoff.Header[0] = FLASH_HEADER_1;
	flash_K9B_onoff.Header[1] = FLASH_HEADER_2;
	flash_K9B_onoff.Header[2] = FLASH_HEADER_1;
	flash_K9B_onoff.Header[3] = FLASH_HEADER_2;
	for (size_t i = 0; i < NUM_ELEMENT; i++)
	{
		flash_K9B_onoff.K9B_onoff[i].pos_save_onoff_next = 0;
		for (size_t j = 0; j < MAX_NUM_K9ONOFF; j++)
		{
			flash_K9B_onoff.K9B_onoff[i].MacK9B[j] = 0;
			flash_K9B_onoff.K9B_onoff[i].K9B_BtKey[j] = 0;
		}
	}
	RD_save_flash_K9B_onoff();
}

static void init_flash_K9B_onoff(void)
{
	rd_read_flash(KEY_FLASH_K9B_ONOFF, &flash_K9B_onoff.Header[0], sizeof(flash_K9B_onoff));
	if (flash_K9B_onoff.Header[0] != FLASH_HEADER_1 && flash_K9B_onoff.Header[1] != FLASH_HEADER_2 &&
		flash_K9B_onoff.Header[3] != FLASH_HEADER_1 && flash_K9B_onoff.Header[4] != FLASH_HEADER_2)
	{
		init_flash_K9B_onoff_default();
	}
	ESP_LOGI("K9B MANAGER", "init flash K9B onoff");
	for (size_t i = 0; i < NUM_ELEMENT; i++)
	{
		printf("K9B: button index %d\n", i);
		printf("pos_save_onoff_next: %d\n", flash_K9B_onoff.K9B_onoff[i].pos_save_onoff_next);
		for (size_t j = 0; j < flash_K9B_onoff.K9B_onoff[i].pos_save_onoff_next; j++)
		{
			if (flash_K9B_onoff.K9B_onoff[i].MacK9B[j] != 0)
			{
				printf("MAC: mac K9B %lu\n", flash_K9B_onoff.K9B_onoff[i].MacK9B[j]);
			}
		}
		printf("------------------------------------------\n");
	}
}

void init_flash_K9B(void)
{
	init_flash_K9B_onoff();
}

void RD_save_flash_K9B_onoff(void)
{
	rd_write_flash(KEY_FLASH_K9B_ONOFF, &flash_K9B_onoff.Header[0], sizeof(flash_K9B_onoff));
}


uint8_t get_flag_pair_onoff(void)
{
	return Sw_Working_Stt_Val.Pair_KP9OnOff_Flag;
}

void K9B_set_start_pair_onoff(uint8_t button_index)
{
	ESP_LOGI("K9B MANAGER", "start pair onoff btn index: %d", button_index);
	Sw_Working_Stt_Val.Clock_time_start_pair_onoff = esp_timer_get_time();
	Sw_Working_Stt_Val.Button_ID_OnOff = button_index;
	Sw_Working_Stt_Val.Pair_KP9OnOff_Flag = 0x01;
}


void K9B_Pair_OnOff_ClearFlag(void)
{
	Sw_Working_Stt_Val.Button_ID_OnOff = 0xff;
	Sw_Working_Stt_Val.Pair_KP9OnOff_Flag = 0;
}


static void K9B_TimeOutScan_OnOff(void)
{
	if (Sw_Working_Stt_Val.Pair_KP9OnOff_Flag == 1 && (esp_timer_get_time() - Sw_Working_Stt_Val.Clock_time_start_pair_onoff >= TIME_OUT_SCAN_KP9))
	{
		ESP_LOGE("KP9 MANAGER", "time out scan onoff");
		K9B_Pair_OnOff_ClearFlag();
	}
}

void K9B_loop_check_pair_time_out(void)
{
	K9B_TimeOutScan_OnOff();
}

/*----------------------------onoff----------------------------------*/
int8_t check_mac_onoff_save_yet(uint8_t index, uint32_t mac)
{
	int8_t pos = -1;
	for (size_t i = 0; i < MAX_NUM_K9ONOFF; i++)
	{
		if (flash_K9B_onoff.K9B_onoff[index].MacK9B[i] == mac)
		{
			pos = i;
		}
	}
	return pos;
}

void save_data_onoff_one_btn(uint8_t index, uint32_t mac, uint8_t key)
{
	uint8_t pos_save_next = flash_K9B_onoff.K9B_onoff[index].pos_save_onoff_next;

	if (pos_save_next < MAX_NUM_K9ONOFF)
	{
		flash_K9B_onoff.K9B_onoff[index].MacK9B[pos_save_next] = mac;
		flash_K9B_onoff.K9B_onoff[index].K9B_BtKey[pos_save_next] = key;
		flash_K9B_onoff.K9B_onoff[index].pos_save_onoff_next++;
	}
	else
	{
		for (size_t i = 0; i < MAX_NUM_K9ONOFF - 1; i++)
		{
			flash_K9B_onoff.K9B_onoff[index].MacK9B[i] = flash_K9B_onoff.K9B_onoff[index].MacK9B[i + 1];
			flash_K9B_onoff.K9B_onoff[index].K9B_BtKey[i] = flash_K9B_onoff.K9B_onoff[index].K9B_BtKey[i + 1];
		}
		flash_K9B_onoff.K9B_onoff[index].MacK9B[MAX_NUM_K9ONOFF - 1] = mac;
		flash_K9B_onoff.K9B_onoff[index].K9B_BtKey[MAX_NUM_K9ONOFF - 1] = key;
	}
	RD_save_flash_K9B_onoff();
}

void delete_onoff_K9B(uint8_t index, uint8_t pos)
{
	uint8_t pos_next = flash_K9B_onoff.K9B_onoff[index].pos_save_onoff_next;
	for (size_t i = pos; i < pos_next; i++)
	{
		flash_K9B_onoff.K9B_onoff[index].MacK9B[i] = flash_K9B_onoff.K9B_onoff[index].MacK9B[i + 1];
		flash_K9B_onoff.K9B_onoff[index].K9B_BtKey[i] = flash_K9B_onoff.K9B_onoff[index].K9B_BtKey[i + 1];
	}
	flash_K9B_onoff.K9B_onoff[index].pos_save_onoff_next--;
	pos_next--;
	flash_K9B_onoff.K9B_onoff[index].MacK9B[pos_next] = 0;
	flash_K9B_onoff.K9B_onoff[index].K9B_BtKey[pos_next] = 0;
	RD_save_flash_K9B_onoff();
}

void K9B_delete_all_onoff_one_btn(uint8_t index)
{
	ESP_LOGE("KP9 MANAGER", "delete all K9B index %d", index);
	if(Sw_Working_Stt_Val.Pair_KP9OnOff_Flag == 0x01){
		K9B_Pair_OnOff_ClearFlag();
		flash_K9B_onoff.K9B_onoff[index].pos_save_onoff_next = 0;
		for (size_t i = 0; i < MAX_NUM_K9ONOFF; i++)
		{
			flash_K9B_onoff.K9B_onoff[index].MacK9B[i] = 0;
			flash_K9B_onoff.K9B_onoff[index].K9B_BtKey[i] = 0;
		}
		RD_save_flash_K9B_onoff();
	}
}

void RD_K9B_Save_OnOff(uint32_t mac, uint8_t key)
{
	if (((1 == key) || (2 == key) || (4 == key) || (8 == key) || (16 == key) || (32 == key) || (40 == key)))
	{
		if (Sw_Working_Stt_Val.Pair_KP9OnOff_Flag == 0x01 && Sw_Working_Stt_Val.Button_ID_OnOff < NUM_ELEMENT)
		{
			int8_t pos_check_mac = check_mac_onoff_save_yet(Sw_Working_Stt_Val.Button_ID_OnOff, mac);
			if (pos_check_mac == -1)
			{
				ESP_LOGW("K9B", "save data onoff, mac: %lu, key: %02x", mac, key);
				save_data_onoff_one_btn(Sw_Working_Stt_Val.Button_ID_OnOff, mac, key);
			}
			else
			{
				ESP_LOGE("K9B", "del data onoff, mac: %lu, key: %02x", mac, key);
				delete_onoff_K9B(Sw_Working_Stt_Val.Button_ID_OnOff, pos_check_mac);
			}
			K9B_Pair_OnOff_ClearFlag();
		}
	}
}

static uint8_t K9B_scan_press_onoff_one_btn(uint8_t btn_index, uint32_t mac, uint8_t key, uint32_t counter, uint8_t type_k9b)
{
	uint8_t stt_return = 0;
	static struct button_value_onoff btn_pair_onoff[NUM_ELEMENT] = {0};
	if (btn_pair_onoff[btn_index].last_counter_onoff == 0x00 || btn_pair_onoff[btn_index].last_counter_onoff != counter)
	{

		for (uint8_t i = 0; i < flash_K9B_onoff.K9B_onoff[btn_index].pos_save_onoff_next; i++)
		{
			if ((key == flash_K9B_onoff.K9B_onoff[btn_index].K9B_BtKey[i] || (key == 0x01 && type_k9b == 6)) && mac == flash_K9B_onoff.K9B_onoff[btn_index].MacK9B[i])
			{
				// ESP_LOGW("K9B","ELEMENT: %d, key: %02x, type deivice: %02x", btn_index, key, type_k9b);
				if (esp_timer_get_time() - btn_pair_onoff[btn_index].time_tongle_onoff >= TIME_OUT_PRESS)
				{
					if (key == 0x01 && type_k9b == 6)
					{
						// off all
						stt_return = 0xff;
					}
					else
					{
						// tongle;
						uint8_t stt = !get_stt_present(btn_index);
						control_set_onoff(btn_index, stt);
			
						stt_return = 1;
					}
					btn_pair_onoff[btn_index].last_counter_onoff = counter;
					btn_pair_onoff[btn_index].time_tongle_onoff = esp_timer_get_time();
				}
			}
		}
	}
	return stt_return;
}

uint8_t RD_K9B_ScanPress_K9BOnOff(uint32_t mac, uint8_t key, uint32_t counter, uint8_t type_k9b)
{
	uint8_t scan_stt = 0;
	if ((key & 0x01) && type_k9b == 6)
		key = 0x01;
	for (uint8_t i = 0; i < NUM_ELEMENT; i++)
	{
		scan_stt = K9B_scan_press_onoff_one_btn(i, mac, key, counter, type_k9b);
	}
	return scan_stt;
}


static void K9B_task(void *arg)
{
	rd_ble_adv_t rx_data_adv;
	init_flash_K9B();
	// esp_task_wdt_add(NULL);
	for (;;)
	{
		// esp_task_wdt_reset();
		if (xQueueReceive(adv_queue_ble, &rx_data_adv, portMAX_DELAY)) // pdMS_TO_TICKS(1), portMAX_DELAY
		{
			switchKP9_proxy = (switchKP9_proxy_t *)(&rx_data_adv.buf[0]);
			static uint32_t count_last = 0;
			static uint32_t mac_last = 0;

			uint32_t macaddr = (rx_data_adv.mac[3] << 24) | (rx_data_adv.mac[2] << 16) | (rx_data_adv.mac[1] << 8) | rx_data_adv.mac[0];
			if (((switchKP9_proxy->counter != count_last) || (mac_last != macaddr)) && ((switchKP9_proxy->key & 0x80) != 0x80))
			{
				// printf("mac: %02X-%02X-%02X-%02X-%02X-%02X---rssi: %d\n", rx_data_adv.mac[5], rx_data_adv.mac[4], rx_data_adv.mac[3],
				// 	   rx_data_adv.mac[2], rx_data_adv.mac[1], rx_data_adv.mac[0], rx_data_adv.rssi);
				// printf("counter : %lu, key value: %02X \n", switchKP9_proxy->counter, switchKP9_proxy->key);

				count_last = switchKP9_proxy->counter;
				mac_last = macaddr;
				RD_K9B_Save_OnOff(macaddr, switchKP9_proxy->key & ~0x80);
				RD_K9B_ScanPress_K9BOnOff(macaddr, switchKP9_proxy->key, switchKP9_proxy->counter, switchKP9_proxy->type_device);		
			}
		}
	}
}
void K9B_init_task(void)
{
	xTaskCreate(K9B_task, "KP9_task", 2048 * 2, NULL, 5, NULL);
}