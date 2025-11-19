/* MQTT (over TCP) Example

	 This example code is in the Public Domain (or CC0 licensed, at your option.)

	 Unless required by applicable law or agreed to in writing, this
	 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
	 CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
// #include "protocol_examples_common.h"


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"
#include "mqtt.h"
#include "err_code.h"
#include "tb.h"
#include "LC8823.h"


static const char *TAG = "mqtt_eth";

static mqtt_callback_driver_t *mqtt_callback_driver;
static esp_mqtt_client_handle_t client;
static volatile bool is_connected = false;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
	ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
	esp_mqtt_event_handle_t event = event_data;
	// esp_mqtt_client_handle_t client = event->client;
	// int msg_id;
	switch ((esp_mqtt_event_id_t)event_id)
	{
	case MQTT_EVENT_CONNECTED:
		ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
		is_connected = true;
		config_stt_connect_mqtt(is_connected);
		if (mqtt_callback_driver && mqtt_callback_driver->on_connected)
		{
			mqtt_callback_driver->on_connected(0);
		}
		break;
	case MQTT_EVENT_DISCONNECTED:
		ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
		is_connected = false;
		config_stt_connect_mqtt(is_connected);
		if (mqtt_callback_driver && mqtt_callback_driver->on_disconnected)
		{
			mqtt_callback_driver->on_disconnected(0);
		}
		break;

	case MQTT_EVENT_SUBSCRIBED:
		ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
		if (mqtt_callback_driver && mqtt_callback_driver->on_subscribed)
		{
			mqtt_callback_driver->on_subscribed(event->msg_id, 0);
		}
		break;
	case MQTT_EVENT_UNSUBSCRIBED:
		ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
		if (mqtt_callback_driver && mqtt_callback_driver->on_unsubscribed)
		{
			mqtt_callback_driver->on_unsubscribed(event->msg_id, 0);
		}
		break;
	case MQTT_EVENT_PUBLISHED:
		ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
		if (mqtt_callback_driver && mqtt_callback_driver->on_published)
		{
			mqtt_callback_driver->on_published(event->msg_id, 0);
		}
		break;
	case MQTT_EVENT_DATA:
		ESP_LOGD(TAG, "MQTT_EVENT_DATA");
		{
			char *topic = malloc(event->topic_len + 1);
			strncpy(topic, event->topic, event->topic_len);
			topic[event->topic_len] = '\0';
			if (event->topic_len > 0 && mqtt_callback_driver && mqtt_callback_driver->on_data)
			{
				mqtt_callback_driver->on_data(event->msg_id, topic, event->data, event->data_len);
			}
			free(topic);
		}
		break;
	case MQTT_EVENT_ERROR:
		ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
		if (mqtt_callback_driver && mqtt_callback_driver->on_state)
		{
			mqtt_callback_driver->on_state(CODE_ERR);
		}
		if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
		{
			ESP_LOGE(TAG, "Last error %s: 0x%x", "reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
			ESP_LOGE(TAG, "Last error %s: 0x%x", "reported from tls stack", event->error_handle->esp_tls_stack_err);
			ESP_LOGE(TAG, "Last error %s: 0x%x", "captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
			ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
		}
		break;
	default:
		ESP_LOGI(TAG, "Other event id:%d", event->event_id);
		break;
	}
}

int mqtt_connect(const Config_t config)
{
	ESP_LOGI(TAG, "mqtt_connect");
#if 0
	esp_mqtt_client_config_t mqtt_cfg = {
			.broker.address.hostname = "demo.thingsboard.io",
			.broker.address.port = 1883,
			.broker.address.transport =0 ? MQTT_TRANSPORT_OVER_SSL : MQTT_TRANSPORT_OVER_TCP,
			.credentials.client_id = username,
			.credentials.username = username,
			.credentials.authentication.password = password,
			.buffer.size = 6000,
	};
#else
	esp_mqtt_client_config_t mqtt_cfg = {
			.broker.address.hostname = config.host,
			.broker.address.port = config.port,
			.broker.address.transport = config.tls ? MQTT_TRANSPORT_OVER_SSL : MQTT_TRANSPORT_OVER_TCP,
			.credentials.client_id = config.clientId,
			.credentials.username = config.username,
			.credentials.authentication.password = config.password,
			.buffer.size = 6000,
	};
#endif

	client = esp_mqtt_client_init(&mqtt_cfg);
	/* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
	esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
	esp_mqtt_client_start(client);
	return CODE_OK;
}

int mqtt_disconnect()
{
	esp_mqtt_client_stop(client);
	return CODE_ERR;
}

bool mqtt_is_connected()
{
	return is_connected;
}

int mqtt_subscribe(int *msg_id, char *topic)
{
	if (is_connected)
	{
		int rs = esp_mqtt_client_subscribe(client, topic, 0);
		if (rs >= 0)
		{
			if (msg_id)
				*msg_id = rs;
			return CODE_OK;
		}
	}
	return CODE_ERR;
}

int mqtt_unsubscribe(int *msg_id, char *topic)
{
	if (is_connected)
	{
		int rs = esp_mqtt_client_unsubscribe(client, topic);
		if (rs >= 0)
		{
			if (msg_id)
				*msg_id = rs;
			return CODE_OK;
		}
	}
	return CODE_ERR;
}

int mqtt_publish(int *msg_id, char *topic, char *data, unsigned int len)
{
	if (is_connected)
	{
		int rs = esp_mqtt_client_publish(client, topic, data, len, 0, 0);
		if (rs >= 0)
		{
			if (msg_id)
				*msg_id = rs;
			return CODE_OK;
		}
	}else{
		ESP_LOGW("MQTT","mqtt hasn't connected yet");
	}
	return CODE_ERR;
}

void mqtt_register_callback_driver(mqtt_callback_driver_t *_mqtt_callback_driver)
{
	mqtt_callback_driver = _mqtt_callback_driver;
}
