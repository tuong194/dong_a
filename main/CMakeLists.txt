file(GLOB srcs
    "device/*.c"
    "K9B_Remote/*.c"
    "provisioning_wifi/*.c"
    "ble/*.c" 
    "rd_config/*.c"
    "controller/*.c"
    "peripheral/*.c"
    "wifi/*.c"
    "mqtt/*.c" 
    "ota/*.c"
    "tb/*.c"
    "util/*.c" 
    "rd_config/*.c"
    "*.c" 
)

set(incs
    "device"
    "K9B_Remote"
    "provisioning_wifi"
    "ble"
    "rd_config"
    "controller"
    "peripheral"
	"wifi"
    "mqtt" 
    "ota"
    "tb"
    "util" 
    "rd_config"
    "."
)
 
idf_component_register(
    SRCS  "${srcs}"
    INCLUDE_DIRS "${incs}"
    REQUIRES "${requires}" 
    #REQUIRES mbedtls esp_srp
)

# spiffs_create_partition_image(storage ../storage FLASH_IN_PROJECT)