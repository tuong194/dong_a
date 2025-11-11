/*
 * SPDX-FileCopyrightText: 2022-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C"
{
#endif

	void wifi_ap_init(void);
	void wifi_init_sta();

#ifdef __cplusplus
}
#endif
