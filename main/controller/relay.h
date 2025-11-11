#ifndef _RELAY_H__
#define _RELAY_H__

#include "stdint.h"
#include "stdbool.h"

#define NUM_CHECK_DETECH_MAX 150
#define TIME_DETECT_ON 6000  // us
#define TIME_DETECT_OFF 4800 // us

typedef struct{
    bool flag_check_control;
    uint8_t stt_relay ;
}relay_stt_t;

void set_stt_relay(uint8_t index, uint8_t stt);
void relay_manager_loop(void);

#endif /* RELAY_H_ */
