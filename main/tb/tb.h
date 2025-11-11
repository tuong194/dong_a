#ifndef TB_H__
#define TB_H__

#include "mqtt.h"
#include <cJSON.h>
#include <sys/queue.h>  //STAILQ

#define tb_status_t int

typedef void (*data_handler_func)(char *topic, char *data, int data_len);
typedef struct
{
	char *topic;
	data_handler_func handler;
	int msg_id;
	bool state;
} sub_topic_t;

typedef int (*rpc_callback_func)(cJSON *reqJson, cJSON *respJson);
typedef struct rpc_register
{
	char cmd[64];
	rpc_callback_func func;
	STAILQ_ENTRY(rpc_register) entries;
} rpc_register_t;

void tb_init();
int tb_connect(const char *username, const char *password);
int tb_mqtt_disconnect();
bool tb_is_connected();

tb_status_t tb_publish_attributes_gateway(char *data);
tb_status_t tb_publish_dev_add_gate(char *type, char *name);

tb_status_t tb_publish_string(char *topic, char *data);
tb_status_t tb_publish_attributes(char *data);
tb_status_t tb_publish_telemetry(char *data);
tb_status_t tb_publish_rpc(char *data);

void tb_register_rpc_callback(const char *cmd, rpc_callback_func rpc_callback);

#endif