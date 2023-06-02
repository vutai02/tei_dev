
#ifndef _IOT_TOUCHPAD_DEBUG_H_
#define _IOT_TOUCHPAD_DEBUG_H_

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/touch_pad.h"
#include "esp_log.h"


#define TOUCH_PAD_NO_CHANGE   (-1)
#define TOUCH_THRESH_NO_USE   (0)
#define TOUCH_FILTER_MODE_EN  (1)
#define TOUCHPAD_FILTER_TOUCH_PERIOD (10)

#ifdef __cplusplus
extern "C" {
#endif

void init_touch();

#ifdef __cplusplus
}
#endif

#endif