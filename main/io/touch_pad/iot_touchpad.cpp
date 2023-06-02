#include "iot_touchpad.h"

CTouchPad::CTouchPad(touch_pad_t touch_pad_num, float sensitivity)
{
  m_tp_handle = iot_tp_create(touch_pad_num, sensitivity);
}

CTouchPad::~CTouchPad()
{
  iot_tp_delete(m_tp_handle);
  m_tp_handle = NULL;
}

esp_err_t CTouchPad::add_cb(tp_cb_type_t cb_type, tp_cb cb, void  *arg)
{
  return iot_tp_add_cb(m_tp_handle, cb_type, cb, arg);
}

esp_err_t CTouchPad::subscribe_event(uint16_t event_mask)
{
  return iot_subscribe_event(m_tp_handle, event_mask);
}

void CTouchPad::set_method_dispatch(touch_dispatch_t dispatch)
{
  iot_tp_set_method_dispatch(m_tp_handle, dispatch);
}

esp_err_t CTouchPad::add_custom_cb(uint32_t press_sec, tp_cb cb, void  *arg)
{
  return iot_tp_add_custom_cb(m_tp_handle, press_sec, cb, arg);
}

touch_pad_t CTouchPad::tp_num()
{
  return iot_tp_num_get(m_tp_handle);
}

esp_err_t CTouchPad::set_threshold(float threshold)
{
  return iot_tp_set_threshold(m_tp_handle, threshold);
}

esp_err_t CTouchPad::get_threshold(float *threshold)
{
  return iot_tp_get_threshold(m_tp_handle, threshold);
}

uint16_t CTouchPad::value()
{
  uint16_t tp_value = 0;
  iot_tp_read(m_tp_handle, &tp_value);
  return tp_value;
}

uint16_t CTouchPad::value_raw()
{
  uint16_t tp_value = 0;
  tp_read_raw(m_tp_handle, &tp_value);
  return tp_value;
}