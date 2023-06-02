#ifndef _IOT_TOUCHPAD_H_
#define _IOT_TOUCHPAD_H_
#include "driver/touch_pad.h"
#include "esp_log.h"
#include "sdkconfig.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void *tp_handle_t;

typedef void (* tp_cb)(void *);           /**< callback function of touchpad */

typedef enum {
  TB_TYPE_PUSH = 1,        /**< touch pad push callback */
  TB_TYPE_RELEASE,         /**< touch pad release callback */
  TB_TYPE_TAP,             /**< touch pad quick tap callback */
  TB_TYPE_LONGPRESS,
  TB_TYPE_MAX,
} tp_cb_type_t;

typedef enum {
  TOUCH_DISP_EVENT,           //!< Event queue dispatch
  TOUCH_DISP_CALLBACK,        //!< Callback dispatch
  TOUCH_DISP_MAX
} touch_dispatch_t;

typedef struct {
  touch_pad_t num;
  tp_cb_type_t type;                      //!< Touch element type
  uint8_t child_msg[8];                   //!< Encoded message
} touch_message_t;

/**
  * @brief create single button device
  *
  * @param touch_pad_num Reference touch_pad_t structure
  * @param sensitivity The max change rate of the reading value when a touch event occurs.
  *         i.e., (non-trigger value - trigger value) / non-trigger value.
  *         Decreasing this threshold appropriately gives higher sensitivity.
  *         If the value is less than 0.1 (10%), leave at least 4 decimal places.
  *
  * @return
  *     tp_handle_t: Touch pad handle.
  */
tp_handle_t iot_tp_create(touch_pad_t touch_pad_num, float sensitivity);

/**
  * @brief delete touchpad device
  *
  * @param tp_handle Touch pad handle.
  *
  * @return
  *     - ESP_OK: succeed
  *     - ESP_FAIL: the param tp_handle is NULL
  */
esp_err_t iot_tp_delete(tp_handle_t tp_handle);

/**
  * @brief add callback function
  *
  * @param tp_handle Touch pad handle.
  * @param cb_type the type of callback to be added
  * @param cb the callback function
  * @param arg the argument of callback function
  *
  * @return
  *     - ESP_OK: succeed
  *     - ESP_FAIL: fail
  */
esp_err_t iot_tp_add_cb(tp_handle_t tp_handle, tp_cb_type_t cb_type, tp_cb cb, void  *arg);
/**
  * @brief add custom callback function
  *
  * @param tp_handle Touch pad handle.
  * @param press_sec the callback function would be called after pointed seconds
  * @param cb the callback function
  * @param arg the argument of callback function
  *
  * @return
  *     - ESP_OK: succeed
  *     - ESP_FAIL: fail
  */
esp_err_t iot_tp_add_custom_cb(tp_handle_t tp_handle, uint32_t press_sec, tp_cb cb, void  *arg);

/**
  * @brief get the number of a touchpad
  *
  * @param tp_handle Touch pad handle.
  *
  * @return touchpad number
  */
touch_pad_t iot_tp_num_get(tp_handle_t tp_handle);

/**
  * @brief Set the trigger threshold of touchpad.
  *
  * @param tp_handle Touch pad handle.
  * @param threshold Should be less than the max change rate of touch.
  *
  * @return
  *     - ESP_OK: succeed
  *     - ESP_FAIL: the param tp_handle is NULL
  */
esp_err_t iot_tp_set_threshold(tp_handle_t tp_handle, float threshold);

/**
  * @brief Get the trigger threshold of touchpad.
  *
  * @param tp_handle Touch pad handle.
  * @param threshold value
  *
  * @return
  *     - ESP_OK: succeed
  *     - ESP_FAIL: the param tp_handle is NULL
  */
esp_err_t iot_tp_get_threshold(const tp_handle_t tp_handle, float *threshold);

/**
  * @brief Get the IIR filter interval of touch sensor when touching.
  *
  * @param tp_handle Touch pad handle.
  * @param filter_ms
  *
  * @return
  *     - ESP_OK: succeed
  *     - ESP_FAIL: the param tp_handle is NULL
  */
esp_err_t iot_tp_get_touch_filter_interval(const tp_handle_t tp_handle, uint32_t *filter_ms);

/**
  * @brief Get the IIR filter interval of touch sensor when idle.
  *
  * @param tp_handle Touch pad handle.
  * @param filter_ms
  *
  * @return
  *     - ESP_OK: succeed
  *     - ESP_FAIL: the param tp_handle is NULL
  */
esp_err_t iot_tp_get_idle_filter_interval(const tp_handle_t tp_handle, uint32_t *filter_ms);

/**
  * @brief Get filtered touch sensor counter value from IIR filter process.
  *
  * @param tp_handle Touch pad handle.
  * @param touch_value_ptr pointer to the value read
  *
  * @return
  *     - ESP_OK: succeed
  *     - ESP_FAIL: the param tp_handle is NULL
  */
esp_err_t iot_tp_read(tp_handle_t tp_handle, uint16_t *touch_value_ptr);

/**
  * @brief Get raw touch sensor counter value from IIR filter process.
  *
  * @param tp_handle Touch pad handle.
  * @param touch_value_ptr pointer to the value read
  *
  * @return
  *     - ESP_OK: succeed
  *     - ESP_FAIL: the param tp_handle is NULL
  */
esp_err_t tp_read_raw(const tp_handle_t tp_handle, uint16_t *touch_value_ptr);


esp_err_t iot_subscribe_event(const tp_handle_t tp_handle, int16_t event_mask);

esp_err_t touch_message_receive(touch_message_t *element_message, uint32_t ticks_to_wait);

esp_err_t iot_tp_set_method_dispatch(tp_handle_t tp_handle, touch_dispatch_t dispatch);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

/**
 * class of touchpad
 */
class CTouchPad
{
private:
  tp_handle_t m_tp_handle;

  /**
   * prevent copy construct
   */
  CTouchPad(const CTouchPad &);
  CTouchPad &operator = (const CTouchPad &);
public:
  /**
    * @brief constructor of CTouchPad
    *
    * @param touch_pad_num Reference touch_pad_t structure
    * @param sensitivity Store the max change rate of the reading value when a touch event occurs.
    *         i.e., (non-trigger value - trigger value) / non-trigger value.
    *         Decreasing this threshold appropriately gives higher sensitivity.
    *         If the value is less than 0.1 (10%), leave at least 4 decimal places.
    *
    */
  CTouchPad(touch_pad_t touch_pad_num, float sensitivity = 0.2);

  ~CTouchPad();

  /**
    * @brief add callback function
    *
    * @param cb_type the type of callback to be added
    * @param cb the callback function
    * @param arg the argument of callback function
    *
    * @return
    *     - ESP_OK: succeed
    *     - ESP_FAIL: fail
    */
  esp_err_t add_cb(tp_cb_type_t cb_type, tp_cb cb, void  *arg);


  esp_err_t subscribe_event(uint16_t event_mask);

  void set_method_dispatch(touch_dispatch_t dispatch);
  /**
    * @brief add custom callback function
    *
    * @param press_sec the callback function would be called after pointed seconds
    * @param cb the callback function
    * @param arg the argument of callback function
    *
    * @return
    *     - ESP_OK: succeed
    *     - ESP_FAIL: fail
    */
  esp_err_t add_custom_cb(uint32_t press_sec, tp_cb cb, void  *arg);

  /**
    * @brief get the number of a touchpad
    *
    * @return touchpad number
    */
  touch_pad_t tp_num();

  /**
    * @brief Set the trigger threshold of touchpad.
    *
    * @param threshold Should be less than the max change rate of touch.
    *
    * @return
    *     - ESP_OK: succeed
    *     - ESP_FAIL: the param tp_handle is NULL
    */
  esp_err_t set_threshold(float threshold);

  /**
    * @brief Get the trigger threshold of touchpad.
    *
    * @param threshold value
    *
    * @return
    *     - ESP_OK: succeed
    *     - ESP_FAIL: the param tp_handle is NULL
    */
  esp_err_t get_threshold(float *threshold);

  /**
    * @brief get filtered touch sensor counter value by IIR filter.
    *
    * @return sample value
    */
  uint16_t value();

  /**
    * @brief get raw data .
    *
    * @return sample value
    */
  uint16_t value_raw();
};

#endif

#endif
