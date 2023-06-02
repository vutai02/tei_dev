#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "esp_attr.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/sens_reg.h"
#include <math.h>
#include <string.h>
#include <time.h>
#include "esp_log.h"
#include "iot_touchpad.h"
#include "sdkconfig.h"

#define IOT_CHECK(tag, a, ret)  if(!(a)) {                                  \
    ESP_LOGE(tag,"%s:%d (%s)", __FILE__, __LINE__, __FUNCTION__);           \
    return (ret);                                                           \
    }

#define ERR_ASSERT(tag, param)  IOT_CHECK(tag, (param) == ESP_OK, ESP_FAIL)
#define POINT_ASSERT(tag, param)    IOT_CHECK(tag, (param) != NULL, ESP_FAIL)
#define RES_ASSERT(tag, res, ret)   IOT_CHECK(tag, (res) != pdFALSE, ret)
#define TIMER_CALLBACK_MAX_WAIT_TICK    (0)
#define T_FREE_AND_NULL(ptr) ({                                              \
    free(ptr);                                                               \
    (ptr) = NULL;                                                            \
})

/*************Fixed Parameters********************/
#define TOUCHPAD_FILTER_IDLE_PERIOD                 100     /**< Period of IIR filter in ms when sensor is not touched. */
#define TOUCHPAD_FILTER_TOUCH_PERIOD                10      /**< Period of IIR filter in ms when sensor is being touched.
                                 Shouldn't change this value. */
#define TOUCHPAD_STATE_SWITCH_DEBOUNCE              20      /**< 20ms; Debounce threshold. */
#define TOUCHPAD_BASELINE_RESET_COUNT_THRESHOLD     10       /**< 5 count number; All channels; */
#define TOUCHPAD_BASELINE_UPDATE_COUNT_THRESHOLD    800     /**< 800ms; Baseline update cycle. */
#define TOUCHPAD_TOUCH_LOW_SENSE_THRESHOLD          0.015    /**< 3% ; Set the low sensitivity threshold.
                                 When less than this threshold, remove the jitter processing. */
#define TOUCHPAD_TOUCH_THRESHOLD_PERCENT            0.9    /**< 75%; This is button type triggering threshold, should be larger than noise threshold.
                                 The threshold determines the sensitivity of the touch. */
#define TOUCHPAD_NOISE_THRESHOLD_PERCENT            0.50    /**< 20%; The threshold is used to determine whether to update the baseline.
                                 The touch system has a signal-to-noise ratio of at least 5:1. */
#define TOUCHPAD_HYSTERESIS_THRESHOLD_PERCENT       0.10    /**< 10%; The threshold prevents frequent triggering. */
#define TOUCHPAD_BASELINE_RESET_THRESHOLD_PERCENT   0.20    /**< 20%; If the touch data exceed this threshold
                                 for 'RESET_COUNT_THRESHOLD' times, then reset baseline to raw data. */

typedef struct tp_custom_cb tp_custom_cb_t;
typedef enum {
  TP_STATE_IDLE = 0,
  TP_STATE_PUSH,
  TP_STATE_PRESS,
  TP_STATE_RELEASE,
} tp_status_t;

typedef struct {
  uint32_t event_mask; 
  tp_cb cb;
  void *arg;
} tp_cb_t;

typedef struct {
  touch_pad_t touch_pad_num;  //Touch pad channel.
  tp_status_t state;          //The button touch status.
  float touchChange;          //User setting. Stores the rate of touch data changes when touched.
  float diff_rate;            //diff_rate = value change of touch / baseline.
  float touch_thr;            //Touch trigger threshold.
  float noise_thr;            //Basedata update threshold.
  float hysteresis_thr;       //The threshold prevents frequent triggering.
  float baseline_reset_thr;   //Basedata reset threshold.
  uint32_t filter_value;      //IIR filter period when touching.
  uint32_t sum_ms;            //Long press parameter.
  uint16_t baseline;          //Base data update from filtered data. solve temperature drift.
  uint16_t debounce_count;    //Debounce count variable.
  uint16_t debounce_th;       //Debounce threshold. If exceeded, confirm the trigger.
  uint16_t bl_reset_count;    //Basedata reset count variable.
  uint16_t bl_reset_count_th; //Basedata reset threshold. If exceeded, reset basedata.
  uint16_t bl_update_count;   //Basedata update count variable.
  uint16_t bl_update_count_th;//Basedata update threshold. If exceeded, update basedata.

  uint32_t serial_thres_sec;  //Continuously triggered threshold parameters.
  tp_cb_t *cb_group[TB_TYPE_MAX]; //Stores global variables for each channel parameter.
  tp_custom_cb_t *custom_cbs; //User-defined callback function.
  uint16_t event_mask;
  touch_dispatch_t method_dispatch;
} tp_dev_t;

struct tp_custom_cb {
  tp_cb cb;
  void *arg;
  TimerHandle_t tmr;
  tp_dev_t *tp_dev;
  tp_custom_cb_t *next_cb;
};

static const char *TAG = "touchpad";        // Debug tag in esp log
static bool g_init_flag = false;            // Judge if initialized the global setting of touch.
static tp_dev_t *tp_group[TOUCH_PAD_MAX];   // Buffer of each button.
static SemaphoreHandle_t s_tp_mux = NULL;
QueueHandle_t event_msg_queue; 

esp_err_t touch_message_receive(touch_message_t *element_message, uint32_t ticks_to_wait)
{
  IOT_CHECK(TAG, element_message != NULL, ESP_ERR_INVALID_ARG);
  IOT_CHECK(TAG, event_msg_queue != NULL, ESP_ERR_INVALID_STATE);
  int ret = xQueueReceive(event_msg_queue, element_message, ticks_to_wait);
  return (ret == pdTRUE) ? ESP_OK : ESP_ERR_TIMEOUT;
}

static void button_event_give(tp_dev_t *tp_dev, tp_cb_type_t cb_type)
{
  if (tp_dev->event_mask & cb_type) {
    touch_message_t message; 
    message.type = cb_type;
    message.num = tp_dev->touch_pad_num;

    int ret = xQueueSend(event_msg_queue, &message, 0);
    if (ret != pdTRUE) {
      ESP_LOGE(TAG, "event queue send failed, event message queue is full");
      return;
    }
  }
}

/* check and run the hooked callback function */
static inline void button_dispatch(tp_dev_t *tp_dev, touch_dispatch_t dispatch_method, tp_cb_type_t cb_type)
{
  if (dispatch_method == TOUCH_DISP_EVENT) {
    button_event_give(tp_dev, cb_type);  //Event queue
  } else if (dispatch_method == TOUCH_DISP_CALLBACK) {
    if (tp_dev->cb_group[cb_type] != NULL) {
      tp_cb_t *cb_info = tp_dev->cb_group[cb_type];
      cb_info->cb(cb_info->arg);
    }
  }
}

esp_err_t iot_tp_set_method_dispatch(tp_handle_t tp_handle, touch_dispatch_t dispatch)
{
  POINT_ASSERT(TAG, tp_handle);
  tp_dev_t *tp_dev = (tp_dev_t *) tp_handle;
  tp_dev->method_dispatch = dispatch;
  return ESP_OK;
}

static void tp_custom_timer_cb(TimerHandle_t xTimer)
{
  tp_custom_cb_t *custom_cb = (tp_custom_cb_t *) pvTimerGetTimerID(xTimer);
  custom_cb->tp_dev->state = TP_STATE_PRESS;
  custom_cb->cb(custom_cb->arg);
}

/* reset all the customed event timers */
static inline void tp_custom_reset_cb_tmrs(tp_dev_t *tp_dev)
{
  tp_custom_cb_t *custom_cb = tp_dev->custom_cbs;
  while (custom_cb != NULL) {
    if (custom_cb->tmr != NULL) {
      xTimerReset(custom_cb->tmr, portMAX_DELAY);
    }
    custom_cb = custom_cb->next_cb;
  }
}

/* stop all the customed event timers */
static inline void tp_custom_stop_cb_tmrs(tp_dev_t *tp_dev)
{
  tp_custom_cb_t *custom_cb = tp_dev->custom_cbs;
  while (custom_cb != NULL) {
    if (custom_cb->tmr != NULL) {
      xTimerStop(custom_cb->tmr, portMAX_DELAY);
    }
    custom_cb = custom_cb->next_cb;
  }
}

/* Call this function after reading the filter once. This function should be registered. */
void filter_read_cb(uint16_t raw_data[], uint16_t filtered_data[])
{
  int16_t diff_data = 0;
  int8_t action_flag = -1;   // If action, increase the filter interval.
  // Main loop to check every channel raw data.
  for (int i = 0; i < TOUCH_PAD_NUM5; i++) {
    if (tp_group[i] != NULL) {
      tp_dev_t *tp_dev = tp_group[i];
      // Use raw data calculate the diff data. Buttons respond fastly. Frequent button ok.
      diff_data = (int16_t) tp_dev->baseline - (int16_t) raw_data[i];
      tp_dev->diff_rate = (float) diff_data / (float) tp_dev->baseline;
      // IDLE status, wait to be pushed
      if (TP_STATE_IDLE == tp_dev->state 
          || TP_STATE_RELEASE == tp_dev->state) {
        
        tp_dev->state = TP_STATE_IDLE;
        // If diff data less than noise threshold, update baseline value.
        if (fabs(tp_dev->diff_rate) <= tp_dev->noise_thr) {
          tp_dev->bl_reset_count = 0; // Clean baseline reset count.
          tp_dev->debounce_count = 0; // Clean debounce count.
          // bl_update_count_th control the baseline update frequency
          if (++tp_dev->bl_update_count > tp_dev->bl_update_count_th) {
            if (-1 == action_flag) {
              action_flag = false;    // Not exceed action line.
            }
            tp_dev->bl_update_count = 0;
            // Baseline updating can use Jitter filter ?
            tp_dev->baseline = filtered_data[i];
          }
        } else {
          action_flag = true; // Exceed action line, represent change the filter Interval.
          tp_dev->bl_update_count = 0;
          // If the diff data is larger than the touch threshold, touch action be triggered.
          if (tp_dev->diff_rate >= tp_dev->touch_thr + tp_dev->hysteresis_thr) {
            tp_dev->bl_reset_count = 0;
            // Debounce processing.
            if (++tp_dev->debounce_count >= tp_dev->debounce_th \
                || tp_dev->touchChange < TOUCHPAD_TOUCH_LOW_SENSE_THRESHOLD) {
              tp_dev->debounce_count = 0;
              tp_dev->state = TP_STATE_PUSH;
              // run push event cb, reset custom event cb
              button_dispatch(tp_dev, tp_dev->method_dispatch, TB_TYPE_PUSH);
              tp_custom_reset_cb_tmrs(tp_dev);
            }
            // diff data exceed the baseline reset line. reset baseline to raw data.
          } else if (tp_dev->diff_rate <= 0 - tp_dev->baseline_reset_thr) {
            tp_dev->debounce_count = 0;
            // Check that if do the reset action again. reset baseline value to raw data.
            if (++tp_dev->bl_reset_count > tp_dev->bl_reset_count_th) {
              tp_dev->bl_reset_count = 0;
              tp_dev->baseline = raw_data[i];
            }
          } else {
            tp_dev->debounce_count = 0;
            tp_dev->bl_reset_count = 0;
          }
        }
      } else {    // The button is in touched status.
        action_flag = true;
        // The button to be pressed continued. long press.
        if (tp_dev->diff_rate > tp_dev->touch_thr - tp_dev->hysteresis_thr) {
          tp_dev->debounce_count = 0;
          // sum_ms is the total time that the read value is under threshold, which means a touch event is on.
          tp_dev->sum_ms += tp_dev->filter_value;
          // whether this is the exact time that a serial event happens.
          if (tp_dev->serial_thres_sec > 0
              && tp_dev->sum_ms - tp_dev->filter_value < tp_dev->serial_thres_sec * 1000
              && tp_dev->sum_ms >= tp_dev->serial_thres_sec * 1000) {
            tp_dev->state = TP_STATE_PRESS;
            tp_dev->sum_ms = 0;
            button_dispatch(tp_dev, tp_dev->method_dispatch, TB_TYPE_LONGPRESS);
          }
        } else {    // Check the release action.
          //  Debounce processing.
          if (++tp_dev->debounce_count >= tp_dev->debounce_th \
              || fabs(tp_dev->diff_rate) < tp_dev->noise_thr \
              || tp_dev->touchChange < TOUCHPAD_TOUCH_LOW_SENSE_THRESHOLD) {
            tp_dev->debounce_count = 0;
            if (tp_dev->state == TP_STATE_PUSH) {
              button_dispatch(tp_dev, tp_dev->method_dispatch, TB_TYPE_TAP);
            }
            tp_dev->sum_ms = 0; // Clean long press count event.
            tp_dev->state = TP_STATE_RELEASE;
            button_dispatch(tp_dev, tp_dev->method_dispatch, TB_TYPE_RELEASE);
            tp_custom_stop_cb_tmrs(tp_dev);
          }
        }
      }
    }
  }
  // Check the button status and to change the filter period.
  if (true == action_flag) {
    touch_pad_set_filter_period(TOUCHPAD_FILTER_TOUCH_PERIOD);
  } 
  else {
    touch_pad_set_filter_period(TOUCHPAD_FILTER_IDLE_PERIOD);
  }
}

/* Creat a button element, init the element parameter */
tp_handle_t iot_tp_create(touch_pad_t touch_pad_num, float sensitivity)
{
  uint16_t tp_val;
  uint32_t avg = 0;
  uint8_t num = 0;
  if (g_init_flag == false) {
    // global touch sensor hardware init
    s_tp_mux = xSemaphoreCreateMutex();
    IOT_CHECK(TAG, s_tp_mux != NULL, NULL);
    event_msg_queue = xQueueCreate(20, sizeof(touch_message_t));
    IOT_CHECK(TAG, event_msg_queue != NULL, NULL);
    g_init_flag = true;
    touch_pad_init();
    // touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);
    touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V);
    touch_pad_filter_start(TOUCHPAD_FILTER_TOUCH_PERIOD);
    touch_pad_set_filter_read_cb(filter_read_cb);
  }
  IOT_CHECK(TAG, touch_pad_num < TOUCH_PAD_MAX, NULL);
  IOT_CHECK(TAG, sensitivity > 0, NULL);
  if (sensitivity < TOUCHPAD_TOUCH_LOW_SENSE_THRESHOLD) {
    ESP_LOGW(TAG, "The sensitivity (change rate of touch reading) is too low, \
             please improve hardware design and improve touch performance.");
  }
  xSemaphoreTake(s_tp_mux, portMAX_DELAY);
  if (tp_group[touch_pad_num] != NULL) {
    ESP_LOGE(TAG, "touchpad create error! The pad has been used!");
    xSemaphoreGive(s_tp_mux);
    return NULL;
  }
  // Init the target touch pad.
  touch_pad_config(touch_pad_num, 0);
  vTaskDelay(20 / portTICK_PERIOD_MS);    // Wait system into stable status.
  for (int i = 0; i < 128; i++) {
    touch_pad_read(touch_pad_num, &tp_val);
    avg += tp_val;
    ++num;
  }
  tp_val = avg / num; // Calculate the initial value.
  ESP_LOGD(TAG, "tp[%d] initial value: %d\n", touch_pad_num, tp_val);
  // Init the status variable for the touch pad.
  tp_dev_t *tp_dev = (tp_dev_t *) calloc(1, sizeof(tp_dev_t));
  tp_dev->filter_value = TOUCHPAD_FILTER_TOUCH_PERIOD;
  tp_dev->touch_pad_num = touch_pad_num;
  tp_dev->sum_ms = 0;
  tp_dev->serial_thres_sec = 2.5;
  tp_dev->state = TP_STATE_IDLE;
  tp_dev->baseline = tp_val;
  tp_dev->touchChange = sensitivity;
  tp_dev->touch_thr = tp_dev->touchChange * TOUCHPAD_TOUCH_THRESHOLD_PERCENT;
  tp_dev->noise_thr = tp_dev->touch_thr * TOUCHPAD_NOISE_THRESHOLD_PERCENT;
  tp_dev->hysteresis_thr = tp_dev->touch_thr * TOUCHPAD_HYSTERESIS_THRESHOLD_PERCENT;
  tp_dev->baseline_reset_thr = tp_dev->touch_thr * TOUCHPAD_BASELINE_RESET_THRESHOLD_PERCENT;
  tp_dev->debounce_th = TOUCHPAD_STATE_SWITCH_DEBOUNCE / TOUCHPAD_FILTER_TOUCH_PERIOD;
  tp_dev->bl_reset_count_th = TOUCHPAD_BASELINE_RESET_COUNT_THRESHOLD;
  tp_dev->bl_update_count_th = TOUCHPAD_BASELINE_UPDATE_COUNT_THRESHOLD / TOUCHPAD_FILTER_IDLE_PERIOD;
  tp_dev->event_mask = 0;
  tp_dev->method_dispatch = TOUCH_DISP_EVENT;
  tp_group[touch_pad_num] = tp_dev;   // TouchPad device add to list.
  xSemaphoreGive(s_tp_mux);
  return (tp_handle_t) tp_dev;
}

/* Clean a button element and setting. */
esp_err_t iot_tp_delete(tp_handle_t tp_handle)
{
  POINT_ASSERT(TAG, tp_handle);
  tp_dev_t *tp_dev = (tp_dev_t *) tp_handle;
  tp_group[tp_dev->touch_pad_num] = NULL;
  for (int i = 0; i < TB_TYPE_MAX; i++) {
    if (tp_dev->cb_group[i] != NULL) {
      free(tp_dev->cb_group[i]);
      tp_dev->cb_group[i] = NULL;
    }
  }
  tp_custom_cb_t *custom_cb = tp_dev->custom_cbs;
  while (custom_cb != NULL) {
    tp_custom_cb_t *cb_next = custom_cb->next_cb;
    xTimerStop(custom_cb->tmr, portMAX_DELAY);
    xTimerDelete(custom_cb->tmr, portMAX_DELAY);
    custom_cb->tmr = NULL;
    free(custom_cb);
    custom_cb = cb_next;
  }
  tp_dev->custom_cbs = NULL;
  free(tp_handle);
  return ESP_OK;
}

/* Add callback API at each button action. */
esp_err_t iot_tp_add_cb(tp_handle_t tp_handle, tp_cb_type_t cb_type, tp_cb cb, void *arg)
{
  POINT_ASSERT(TAG, tp_handle);
  POINT_ASSERT(TAG, cb);
  IOT_CHECK(TAG, cb_type < TB_TYPE_MAX, ESP_FAIL);
  tp_dev_t *tp_dev = (tp_dev_t *) tp_handle;
  if (tp_dev->cb_group[cb_type] != NULL) {
    ESP_LOGW(TAG, "This type of touchpad callback function has already been added!");
    return ESP_FAIL;
  }
  tp_cb_t *cb_info = (tp_cb_t *) calloc(1, sizeof(tp_cb_t));
  POINT_ASSERT(TAG, cb_info);
  cb_info->cb = cb;
  cb_info->arg = arg;
  tp_dev->cb_group[cb_type] = cb_info;
  return ESP_OK;
}

esp_err_t iot_subscribe_event(const tp_handle_t tp_handle, int16_t event_mask)
{
  POINT_ASSERT(TAG, tp_handle);
  tp_dev_t *tp_dev = (tp_dev_t *) tp_handle;
  tp_dev->event_mask = event_mask;
  return ESP_OK;
}

esp_err_t iot_tp_add_custom_cb(tp_handle_t tp_handle, uint32_t press_sec, tp_cb cb, void  *arg)
{
  POINT_ASSERT(TAG, tp_handle);
  POINT_ASSERT(TAG, cb);
  IOT_CHECK(TAG, press_sec != 0, ESP_FAIL);
  tp_dev_t *tp_dev = (tp_dev_t *) tp_handle;
  tp_custom_cb_t *cb_new = (tp_custom_cb_t *) calloc(1, sizeof(tp_custom_cb_t));
  POINT_ASSERT(TAG, cb_new);
  cb_new->cb = cb;
  cb_new->arg = arg;
  cb_new->tp_dev = tp_dev;
  cb_new->tmr = xTimerCreate("custom_cb_tmr", press_sec * 1000 / portTICK_PERIOD_MS, pdFALSE, cb_new, tp_custom_timer_cb);
  if (cb_new->tmr == NULL) {
    ESP_LOGE(TAG, "timer create fail! %s:%d (%s)", __FILE__, __LINE__, __FUNCTION__);
    free(cb_new);
    return ESP_FAIL;
  }
  cb_new->next_cb = tp_dev->custom_cbs;
  tp_dev->custom_cbs = cb_new;
  return ESP_OK;
}

touch_pad_t iot_tp_num_get(const tp_handle_t tp_handle)
{
  tp_dev_t *tp_dev = (tp_dev_t *) tp_handle;
  return tp_dev->touch_pad_num;
}

esp_err_t iot_tp_set_threshold(const tp_handle_t tp_handle, float threshold)
{
  POINT_ASSERT(TAG, tp_handle);
  tp_dev_t *tp_dev = (tp_dev_t *) tp_handle;
  ERR_ASSERT(TAG, touch_pad_config(tp_dev->touch_pad_num, threshold));
  // updata all the threshold and other related value.
  tp_dev->touch_thr = threshold;
  tp_dev->noise_thr = tp_dev->touch_thr * TOUCHPAD_NOISE_THRESHOLD_PERCENT;
  tp_dev->hysteresis_thr = tp_dev->touch_thr * TOUCHPAD_HYSTERESIS_THRESHOLD_PERCENT;
  tp_dev->baseline_reset_thr = tp_dev->touch_thr * TOUCHPAD_BASELINE_RESET_THRESHOLD_PERCENT;
  return ESP_OK;
}

esp_err_t iot_tp_get_threshold(const tp_handle_t tp_handle, float *threshold)
{
  POINT_ASSERT(TAG, tp_handle);
  tp_dev_t *tp_dev = (tp_dev_t *) tp_handle;
  *threshold = tp_dev->touch_thr;
  return ESP_OK;
}

esp_err_t iot_tp_get_touch_filter_interval(const tp_handle_t tp_handle, uint32_t *filter_ms)
{
  POINT_ASSERT(TAG, tp_handle);
  *filter_ms = TOUCHPAD_FILTER_TOUCH_PERIOD;
  return ESP_OK;
}

esp_err_t iot_tp_get_idle_filter_interval(const tp_handle_t tp_handle, uint32_t *filter_ms)
{
  POINT_ASSERT(TAG, tp_handle);
  *filter_ms = TOUCHPAD_FILTER_IDLE_PERIOD;
  return ESP_OK;
}

esp_err_t iot_tp_read(const tp_handle_t tp_handle, uint16_t *touch_value_ptr)
{
  POINT_ASSERT(TAG, tp_handle);
  tp_dev_t *tp_dev = (tp_dev_t *) tp_handle;
  return touch_pad_read_filtered(tp_dev->touch_pad_num, touch_value_ptr);
}

esp_err_t tp_read_raw(const tp_handle_t tp_handle, uint16_t *touch_value_ptr)
{
  POINT_ASSERT(TAG, tp_handle);
  tp_dev_t *tp_dev = (tp_dev_t *) tp_handle;
  return touch_pad_read_raw_data(tp_dev->touch_pad_num, touch_value_ptr);
}