#ifndef FC278610_3591_4DFC_B68A_F7C40A3573DF
#define FC278610_3591_4DFC_B68A_F7C40A3573DF


#include <stdbool.h>


#define STRING_VALUE_MAX_SIZE 128

#define HOST_KEY "host"
#define PORT_KEY "port"
#define CLIENT_ID_KEY "clientId"
#define USERNAME_KEY "username"
#define PASSWORD_KEY "password"
#define KEEP_ALIVE_KEY "keepAlive"
#define TLS_KEY "tls"
#define HOME_ID_KEY "homeId"
#define ADD_HOME_KEY "addhome"

#define HOST_DEFAULT "demo.thingsboard.io"
#define PORT_DEFAULT 1883
#define CLIENT_ID_DEFAULT "default_client_id"
#define USERNAME_DEFAULT "default_username"
#define PASSWORD_DEFAULT "default_password"
#define KEEP_ALIVE_DEFAULT 60
#define TLS_DEFAULT false
#define HOME_ID_DEFAULT "default_home_id"

typedef struct {
    char host[STRING_VALUE_MAX_SIZE];
    int port;
    char clientId[STRING_VALUE_MAX_SIZE];
    char username[STRING_VALUE_MAX_SIZE]; //access token
    char password[STRING_VALUE_MAX_SIZE];
    int keepAlive;
    bool tls;
    char home_id[STRING_VALUE_MAX_SIZE];
    uint8_t check_add_home;
    char mac[16];
    char ip[32];
    char ssid_config[32];
}Config;

extern Config config;

void Config_Read_Flash();
void Config_Print();

bool Config_SetHost(const char *host);
bool Config_SetPort(int port);
bool Config_SetClientId(const char *clientId);
bool Config_SetUsername(const char *username);
bool Config_SetPassword(const char *password);
bool Config_SetKeepAlive(int keepAlive);
bool Config_SetTLS(bool tls);
bool Config_SetHomeId(const char *homeId);
bool Config_Set_addHome(bool add);
uint8_t inline check_add_home(void) {
    return config.check_add_home;
}

#endif /* FC278610_3591_4DFC_B68A_F7C40A3573DF */
