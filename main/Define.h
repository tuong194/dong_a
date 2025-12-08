#ifndef _RD_DEFINE_H__
#define _RD_DEFINE_H__

#include "soc/gpio_num.h"

#define TICK_INTERVAL    10 //ms

#define OFF_STT    0
#define ON_STT     1

#define NUM_ELEMENT   4
#define BTN_NUM       NUM_ELEMENT
#define RELAY_NUM     NUM_ELEMENT

#define TOUCH_ACTIVE_POW			0
#define TOUCH_NON_ACTIVE_POW		1

#define TOUCH                       1
#define NO_TOUCH                    0

#define RESET_TOUCH_PIN  GPIO_NUM_1  
#define DETECT_ZERO_PIN  GPIO_NUM_6 

#define LED_DATA         GPIO_NUM_21   
#define LED_CLK          GPIO_NUM_20   

#define ALL_LED       0xff

#if NUM_ELEMENT >= 1
    #define ELE_1         0
    #define BTN_1         ELE_1
    #define BUTTON_PIN1   GPIO_NUM_8
    #define	RELAY1_PIN	  GPIO_NUM_10	
#endif

#if NUM_ELEMENT >= 2
    #define ELE_2         1
    #define BTN_2         ELE_2
    #define BUTTON_PIN2   GPIO_NUM_3
    #define	RELAY2_PIN    GPIO_NUM_4
#endif

#if NUM_ELEMENT >= 3
    #define ELE_3         2
    #define BTN_3         ELE_3
    #define BUTTON_PIN3   GPIO_NUM_7
    #define	RELAY3_PIN	  GPIO_NUM_5	
#endif

#if NUM_ELEMENT >= 4
    #define ELE_4         3
    #define BTN_4         ELE_4
    #define BUTTON_PIN4   GPIO_NUM_0
    #define	RELAY4_PIN	  GPIO_NUM_6
#endif

#define METHOD_REQUEST_STT_BT "controlDev"
#define METHOD_ADD_DEVICE "registerDeviceWifi"
#define METHOD_DEL_DEVICE "delDeviceWifi"

#define METHOD_RESPONE_STT_BT "controlDevRsp"
#define METHOD_ADD_DEVICE_RSP "registerDeviceWifiRsp"
#define METHOD_DEL_DEVICE_RSP "delDeviceWifiRsp"
#define METHOD_RESPONE_RESET_DEVICE "resetDeviceWifi"

#define KEY_MAC "mac"
#define KEY_TYPE "type"
#define KEY_NAME "name"
#define KEY_VERSION "version"
#define KEY_IP "ip"
#define KEY_NAME_WIFI "nameWifiConnect"
#define KEY_HOME_ID "dormitoryId"

#if NUM_ELEMENT == 1
#define DEVICE_NAME_PREFIX "Congtac1nut_"
#elif NUM_ELEMENT == 2
#define DEVICE_NAME_PREFIX "Congtac2nut_"
#elif NUM_ELEMENT == 3
#define DEVICE_NAME_PREFIX "Congtac3nut_"
#elif NUM_ELEMENT == 4
#define DEVICE_NAME_PREFIX "Congtac4nut_"
#endif

#define DEVICE_TYPE     6262
#define DEVICE_VERSION  PROJECT_VER

#define KEY_COUNTDOWN "countdown"
#define KEY_ONOFF "onoff"
#define KEY_STATUS_STARTUP "statusStartup"

#define KEY_ONOFF_BT1 "bt"
#define KEY_ONOFF_BT2 "bt2"
#define KEY_ONOFF_BT3 "bt3"
#define KEY_ONOFF_BT4 "bt4"

#define KEY_RED_ON_1 "rOn"
#define KEY_BLUE_ON_1 "bOn"
#define KEY_GREEN_ON_1 "gOn"
#define KEY_DIM_ON_1 "dimOn"

#define KEY_RED_OFF_1 "rOff"
#define KEY_BLUE_OFF_1 "bOff"
#define KEY_GREEN_OFF_1 "gOff"
#define KEY_DIM_OFF_1 "dimOff"

#define KEY_RED_ON_2 "rOn2"
#define KEY_BLUE_ON_2 "bOn2"
#define KEY_GREEN_ON_2 "gOn2"
#define KEY_DIM_ON_2 "dimOn2"

#define KEY_RED_OFF_2 "rOff2"
#define KEY_BLUE_OFF_2 "bOff2"
#define KEY_GREEN_OFF_2 "gOff2"
#define KEY_DIM_OFF_2 "dimOff2"

#define KEY_RED_ON_3 "rOn3"
#define KEY_BLUE_ON_3 "bOn3"
#define KEY_GREEN_ON_3 "gOn3"
#define KEY_DIM_ON_3 "dimOn3"

#define KEY_RED_OFF_3 "rOff3"
#define KEY_BLUE_OFF_3 "bOff3"
#define KEY_GREEN_OFF_3 "gOff3"
#define KEY_DIM_OFF_3 "dimOff3"

#define KEY_RED_ON_4 "rOn4"
#define KEY_BLUE_ON_4 "bOn4"
#define KEY_GREEN_ON_4 "gOn4"
#define KEY_DIM_ON_4 "dimOn4"

#define KEY_RED_OFF_4 "rOff4"
#define KEY_BLUE_OFF_4 "bOff4"
#define KEY_GREEN_OFF_4 "gOff4"
#define KEY_DIM_OFF_4 "dimOff4"

#endif /*  */
