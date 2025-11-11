#ifndef MQTT_H__
#define MQTT_H__

#include <stdbool.h>
#include "esp_err.h"

typedef struct
{
	void (*on_connected)(int result);
	void (*on_disconnected)(int result);
	void (*on_subscribed)(int msg_id, int result);
	void (*on_unsubscribed)(int msg_id, int result);
	void (*on_published)(int msg_id, int result);
	void (*on_state)(int err_code);
	void (*on_data)(int msg_id, char *topic, char *payload, int payload_len);
} mqtt_callback_driver_t;

int mqtt_connect(const char *username, const char *password);
int mqtt_disconnect();
bool mqtt_is_connected();
int mqtt_subscribe(int *msg_id, char *topic);
int mqtt_unsubscribe(int *msg_id, char *topic);
int mqtt_publish(int *msg_id, char *topic, char *data, unsigned int len);
void mqtt_register_callback_driver(mqtt_callback_driver_t *_mqtt_callback_driver);

#endif