
#ifndef UTIL_H
#define UTIL_H

#include "esp_rom_sys.h"

#define SLEEP_US(us)     esp_rom_delay_us(us)

uint8_t rd_exceed_us(int64_t ref, int64_t span_us);
void get_mac_str(char *mac_str);
char *ConvertToCharLower(char *str);
char *hex_to_ascii(char *hex_string);

#endif