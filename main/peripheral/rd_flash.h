#ifndef RD_FLASH_H_
#define RD_FLASH_H_

#include <stdio.h>

#define FLASH_TYPE_ADDR      0
#define FLASH_TYPE_KEY_VALUE 1
#define TYPE_FLASH           FLASH_TYPE_KEY_VALUE

#define NAME_SPACE              "RDstorage"

#define KEY_FLASH_CONFIG        "RDConfig"
#define KEY_FLASH_SCENE         "RDScene"
#define KEY_FLASH_K9B_ONOFF     "KeyK9Bonoff"
#define KEY_FLASH_K9B_HC        "KeyK9BHC"

#define  FLASH_ADD_CONFIG 0x310000
#define  FLASH_ADD_SCENE  0x312000
#define  FLASH_ADD_GROUP  0x311000

#define FLASH_HEADER_1          0x55
#define FLASH_HEADER_2			0xAA

void rd_write_flash(const char* key, void *data, size_t length);
void rd_read_flash(const char* key, void *data_rec, size_t length);
void rd_flash_erase(const char* key);
size_t get_size(const char* key);
void rd_flash_init(void);

#endif