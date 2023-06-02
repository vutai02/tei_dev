
#include <memory>
#include <driver/gpio.h>
#include <smooth/core/util/ByteSet.h>
#include <smooth/core/ipc/Publisher.h>
#include <smooth/core/util/json_util.h>
#include <smooth/core/task_priorities.h>
#include "utils/timer_id.h"
#include "utils/DataCaChe.hpp"
#include "common/EventLed.hpp"
#include "IoTouchController.h"
#include "common/ConstantType.h"
#include "common/ConfigConstant.h"
#include "common/ObjectTypeButton.h"
#include "storage/nvs/StorageNvsE.h"
#include "common/digital/DigitalInputValue.h"
#include "common/digital/DigitalOutputValue.h"
#include "common/digital/DigitalStatusValue.h"

#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/queue.h"
#include "freertos/task.h"

using namespace std;
using namespace std::chrono;
using namespace iotTouch::common;
using namespace smooth::core::io;
using namespace smooth::core::timer;
using namespace smooth::core::logging;
using namespace smooth::core::util;
using namespace smooth::core::json_util;
using namespace smooth::core::ipc;
using namespace iotTouch::storage;

namespace iotTouch
{
static volatile iot_tp_dev_t tp_group[TOUCH_PAD_MAX];   // Buffer of each button.
static smooth::core::util::ByteSet is_disable_touch(0);
static smooth::core::util::ByteSet is_disabled_led(0);
static constexpr const char* IO_TAG = "IO_tag";
static volatile bool disable_touch = false;
static volatile ModeTouch state_mode_touch = ModeTouch::None;

bool is_enable_interupt = false;
struct Message xMessage;
static QueueHandle_t intr_queue = NULL;

IoTouchControllerTask::IoTouchControllerTask()
    : Task(IO_TAG, 5 * 1024, 4, milliseconds{50}),
    relay_value_(DigitalOutputValueQueue::create(6, *this, *this)),
    led_(DigitalIoLedQueue::create(6, *this, *this)),
    turn_off_led_(DigitalTurnOffLedQueue::create(2, *this, *this)),
    ota_pending_touch_(EventPendingQueue::create(2, *this, *this)),
    state_touch_detail_(EnabledTouchDetailQueue::create(2, *this, *this)),
    sensitivity_touch_(SensitivityTouchValueQueue::create(2, *this, *this)),
    zero_detect_(&IoTouchControllerTask::isr_zero_detect, this, IO_TOUCH_ZERO, true, false, GPIO_INTR_POSEDGE)
{
  #if TOUCH_TYPE >= 1
    out_relay_1_.set(false);
  #if TOUCH_TYPE >= 2
    out_relay_2_.set(false);
  #if TOUCH_TYPE >= 3
    out_relay_3_.set(false);
  #if TOUCH_TYPE >= 4
    out_relay_4_.set(false);
  #endif
  #endif
  #endif
  #endif

  out_relay_.set();
  out_led_.set(false);
  state_mode_touch = ModeTouch::None;
  is_turn_off_led_ = false;

  xMessage.is_start_cache = true;
  for (size_t i = 0; i < TOUCH_TYPE; i++) {
    xMessage.data[i].id = 0; 
    xMessage.data[i].state = 0;
    xMessage.data[i].is_local = true;
    xMessage.data[i].value = false;
  }
  

  intr_queue = xQueueCreate(10, sizeof(xMessage));
  xTaskCreatePinnedToCore(isr_task, "isr_task", 1024 * 3, this, 10, NULL, 1);
}

void IoTouchControllerTask::tick()
{
  if (xMessage.is_start_cache) {
    if (tp_group[TOUCH_BUTTON_1].state) {
      tp_group[TOUCH_BUTTON_1].state = 0;
      xMessage.data[0].state = 1;
      xMessage.data[0].id = tp_group[TOUCH_BUTTON_1].tp;
      xMessage.data[0].value = tp_group[TOUCH_BUTTON_1].value;
      xMessage.data[0].is_local  = tp_group[TOUCH_BUTTON_1].is_ack ? false : true;
      is_enable_interupt = true;
    }
  #if TOUCH_TYPE >= 2
    if (tp_group[TOUCH_BUTTON_2].state) {
      tp_group[TOUCH_BUTTON_2].state = 0;
      xMessage.data[1].state = 1;
      xMessage.data[1].id = tp_group[TOUCH_BUTTON_2].tp;
      xMessage.data[1].value = tp_group[TOUCH_BUTTON_2].value;
      xMessage.data[1].is_local  = tp_group[TOUCH_BUTTON_2].is_ack ? false : true;
      is_enable_interupt = true;
    }
  #endif
  #if TOUCH_TYPE >= 3
    if (tp_group[TOUCH_BUTTON_3].state) {
      tp_group[TOUCH_BUTTON_3].state = 0;
      xMessage.data[2].state = 1;
      xMessage.data[2].id = tp_group[TOUCH_BUTTON_3].tp;
      xMessage.data[2].value = tp_group[TOUCH_BUTTON_3].value;
      xMessage.data[2].is_local  = tp_group[TOUCH_BUTTON_3].is_ack ? false : true;
      is_enable_interupt = true;
    }
  #endif

  #if TOUCH_TYPE >= 4
    if (tp_group[TOUCH_BUTTON_4].state) {
      tp_group[TOUCH_BUTTON_4].state = 0;
      xMessage.data[3].state = 1;
      xMessage.data[3].id = tp_group[TOUCH_BUTTON_4].tp;
      xMessage.data[3].value = tp_group[TOUCH_BUTTON_4].value;
      xMessage.data[3].is_local  = tp_group[TOUCH_BUTTON_4].is_ack ? false : true;
      is_enable_interupt = true;
    }
  #endif

    if (is_enable_interupt) {
      is_enable_interupt = false;
      xMessage.is_start_cache = false;
      gpio_intr_enable(IO_TOUCH_ZERO);
    }
  }
}

static void push_cb(uint16_t num)
{
  if (!disable_touch && !is_disable_touch.test(tp_group[num].tp)) {
    Log::info(IO_TAG, "push_cb");
    tp_group[num].value = !tp_group[num].value;
    tp_group[num].state = 1;
    tp_group[num].is_ack = false;
  }
}

static void release_cb()
{
  switch (state_mode_touch)
  {
  case ModeTouch::Smart:
    Publisher<ObjectTypeButton>::publish(ObjectTypeButton(TypeButton::SMART));
    state_mode_touch = ModeTouch::None;
    break;
  case ModeTouch::Provision:
    Publisher<ObjectTypeButton>::publish(ObjectTypeButton(TypeButton::PROVISIONING));
    state_mode_touch = ModeTouch::None;
    break;
  case ModeTouch::Refactor:
    Publisher<ObjectTypeButton>::publish(ObjectTypeButton(TypeButton::REFACTORY_SYSTEM));
    state_mode_touch = ModeTouch::None;
    break;
  case ModeTouch::None:
    break;
  default:
    break;
  }
}

static void provision_cb(uint16_t num)
{
  if (!disable_touch && iotTouch::DataCache::instance().get(ACTIVATE) == "0") {
    state_mode_touch = ModeTouch::Provision;
    Publisher<led::EventLed>::publish(
      led::EventLed("provisioning")
    );
    Log::info(IO_TAG, "enter provisioning");
  }
}

static void smart_cb(uint16_t num)
{
  if (!disable_touch && iotTouch::DataCache::instance().get(ACTIVATE) == "0") {
    tp_group[num].value = false;
    Publisher<DigitalOutputValue>::publish(
      DigitalOutputValue(
        tp_group[num].tp,
        tp_group[num].value
      )
    );

    state_mode_touch = ModeTouch::Smart;
    if (iotTouch::DataCache::instance().get(WIFI_MODE) 
            != std::to_string(WifiMode::ap_mode)) {
      Publisher<led::EventLed>::publish(
        led::EventLed("exit")
      );
      Log::info(IO_TAG, "");
    }
    else {
      Publisher<led::EventLed>::publish(
        led::EventLed("smart")
      );
      Log::info(IO_TAG, "enter smart");
    }
  }
}

static void refactor_cb(uint16_t num)
{
  state_mode_touch = ModeTouch::Refactor;

  tp_group[num].value = false;
  Publisher<DigitalOutputValue>::publish(
    DigitalOutputValue(
      tp_group[num].tp,
      tp_group[num].value
    )
  );
  
  Publisher<led::EventLed>::publish(
    led::EventLed("refactor")
  );
  Log::info(IO_TAG, "enter Refactor");
}

void IoTouchControllerTask::init()
{
  esp_log_level_set(IO_TAG, static_cast<esp_log_level_t>(TOUCH_IO_LOGGING_LEVEL));

  prepare_hw();
  
  initialize_touch();
}

static void button_handler_task(void *arg)
{
  (void) arg;
  touch_message_t message;
  uint8_t state = 0;
  while (1) {
    if (ESP_OK == touch_message_receive(&message, 100)) {
      switch (message.type) {
      case tp_cb_type_t::TB_TYPE_PUSH:
        push_cb(message.num);
        state = 0;
        break;
      case tp_cb_type_t::TB_TYPE_RELEASE:
        release_cb();
        state = 0;
        break;
      case tp_cb_type_t::TB_TYPE_LONGPRESS:
        state++;
        if (state == 1) {
          // smart
          smart_cb(message.num);
        }
        else if (state == 2) {
          // provision
          provision_cb(message.num);
        }
        else if (state == 3) {
          // refactor
          refactor_cb(message.num);
        }
        break;
      
      default:
        break;
      }
    }
  }
}

void IoTouchControllerTask::initialize_touch()
{

#if TOUCH_TYPE >= 1
  tp_dev1_ = new CTouchPad(TOUCH_BUTTON_1, TOUCH_BUTTON_MAX_CHANGE_RATE_1);
  tp_dev1_->subscribe_event(TB_TYPE_PUSH | TB_TYPE_RELEASE | TB_TYPE_LONGPRESS);
#if TOUCH_TYPE  >= 2
  tp_dev2_ = new CTouchPad(TOUCH_BUTTON_2, TOUCH_BUTTON_MAX_CHANGE_RATE_2);
  tp_dev2_->subscribe_event(TB_TYPE_PUSH | TB_TYPE_RELEASE | TB_TYPE_LONGPRESS);
#if TOUCH_TYPE  >= 3
  tp_dev3_ = new CTouchPad(TOUCH_BUTTON_3, TOUCH_BUTTON_MAX_CHANGE_RATE_3);
  tp_dev3_->subscribe_event(TB_TYPE_PUSH | TB_TYPE_RELEASE | TB_TYPE_LONGPRESS);
#if TOUCH_TYPE  >= 4
  tp_dev4_ = new CTouchPad(TOUCH_BUTTON_4, TOUCH_BUTTON_MAX_CHANGE_RATE_4);
  tp_dev4_->subscribe_event(TB_TYPE_PUSH | TB_TYPE_RELEASE | TB_TYPE_LONGPRESS);
#endif
#endif
#endif
#endif

  xTaskCreate(&button_handler_task, "button_handler_task", 4 * 1024, NULL, 5, NULL);
}


void IoTouchControllerTask::set_io(uint8_t position, bool value)
{
  if (is_disable_touch.test(position))  return;
  switch (position) {
    case 0:
      out_relay_1_.set(value);
      update_storage_cnt_relay("cnt_touch_0");
      break;
#if TOUCH_TYPE >= 2
    case 1:
      out_relay_2_.set(value);
      update_storage_cnt_relay("cnt_touch_1");
      break;
    case 2:
#if TOUCH_TYPE >= 3
      out_relay_3_.set(value);
      update_storage_cnt_relay("cnt_touch_2");
      break;
#if TOUCH_TYPE >= 4
    case 3:
      out_relay_4_.set(value);
      update_storage_cnt_relay("cnt_touch_3");
      break;
#endif
#endif
#endif
    default:
      break;
  }
}

void IoTouchControllerTask::event(const DigitalOutputValue& ev)
{
  switch (ev.get_output())
  {
#if TOUCH_TYPE >= 1
  case 0:
    tp_group[TOUCH_BUTTON_1].state = 1;
    tp_group[TOUCH_BUTTON_1].value = ev.get_value();
    tp_group[TOUCH_BUTTON_1].is_ack = ev.get_local() ? false : true;
    break;
#if TOUCH_TYPE >= 2
  case 1:
    tp_group[TOUCH_BUTTON_2].state = 1;
    tp_group[TOUCH_BUTTON_2].value = ev.get_value();
    tp_group[TOUCH_BUTTON_2].is_ack = ev.get_local() ? false : true;
    break;
#if TOUCH_TYPE >= 3
  case 2:
    tp_group[TOUCH_BUTTON_3].state = 1;
    tp_group[TOUCH_BUTTON_3].value = ev.get_value();
    tp_group[TOUCH_BUTTON_3].is_ack = ev.get_local() ? false : true;
    break;
#if TOUCH_TYPE >= 4
  case 3:
    tp_group[TOUCH_BUTTON_4].state = 1;
    tp_group[TOUCH_BUTTON_4].value = ev.get_value();
    tp_group[TOUCH_BUTTON_4].is_ack = ev.get_local() ? false : true;
    break;
#endif
#endif
#endif
#endif
  case 4:
#if TOUCH_TYPE >= 1
    tp_group[TOUCH_BUTTON_1].state = 1;
    tp_group[TOUCH_BUTTON_1].value = ev.get_value();
    tp_group[TOUCH_BUTTON_1].is_ack = ev.get_local() ? false : true;

#if TOUCH_TYPE >= 2
    tp_group[TOUCH_BUTTON_2].state = 1;
    tp_group[TOUCH_BUTTON_2].value = ev.get_value();
    tp_group[TOUCH_BUTTON_2].is_ack = ev.get_local() ? false : true;
#if TOUCH_TYPE >= 3
    tp_group[TOUCH_BUTTON_3].state = 1;
    tp_group[TOUCH_BUTTON_3].value = ev.get_value();
    tp_group[TOUCH_BUTTON_3].is_ack = ev.get_local() ? false : true;
#if TOUCH_TYPE >= 4
    tp_group[TOUCH_BUTTON_4].state = 1;
    tp_group[TOUCH_BUTTON_4].value = ev.get_value();
    tp_group[TOUCH_BUTTON_4].is_ack = ev.get_local() ? false : true;
#endif
#endif
#endif
#endif
    break;
  default:
    break;
  }
}

void IoTouchControllerTask::event(const DigitalIoLed& ev)
{
  out_led_.set(ev.get_state());
  Log::info(IO_TAG, "DigitalIoLed state: {0}", ev.get_state());
}

void IoTouchControllerTask::event(const DigitalTurnOffLedRed& ev)
{
  out_led_.set(ev.get_state_led());
  Log::info(IO_TAG, "--->>>> Turn off led: {0}", ev.get_state_led());
}


void IoTouchControllerTask::isr_zero_detect(void* context)
{
  struct Message *pToxMessage;
  pToxMessage = &xMessage;
  xQueueSendFromISR(intr_queue, (void *)&pToxMessage, NULL);
}

void IoTouchControllerTask::isr_process(struct Message& msg)
{
  if (gpio_get_level(zero_detect_.get_io()) == 1) {
    gpio_intr_disable(IO_TOUCH_ZERO);
    std::this_thread::sleep_for(std::chrono::milliseconds{6});
    for (size_t i = 0; i < TOUCH_TYPE; i++) {
      if (msg.data[i].state) {
        set_io(msg.data[i].id, msg.data[i].value);
      }
    }

    for (size_t i = 0; i < TOUCH_TYPE; i++) {
      if (msg.data[i].state == 1) {
        Publisher<DigitalInputValue>::publish(DigitalInputValue(
          msg.data[i].id,
          msg.data[i].value,
          !msg.data[i].is_local
        ));

        msg.data[i].state = 0;
        msg.data[i].is_local = true;
      }
    }
    msg.is_start_cache = true;
  }
}

void IoTouchControllerTask::isr_task(void* context)
{
  auto w = reinterpret_cast<IoTouchControllerTask*>(context);
  // struct Message msg;
  struct Message *pRxMessage;
  while(1) {
    if (intr_queue) {
      if (xQueueReceive(intr_queue, &pRxMessage, portMAX_DELAY) == pdPASS) {
        w->isr_process(*pRxMessage);
      }
    }
  }
}

bool IoTouchControllerTask::prepare_hw()
{
  int32_t value_default = 0;
  // set state tp
  for (int i = 0; i < TOUCH_PAD_MAX; i++) {
    tp_group[i].state = false;
  }
  // set position tp 
#if TOUCH_TYPE >= 1
  tp_group[TOUCH_BUTTON_1].tp = 0;
  if (StorageNvsE::instance().read("touch_0", &value_default)) {
    tp_group[TOUCH_BUTTON_1].state = 1;
    tp_group[TOUCH_BUTTON_1].value = value_default == 1 ? true : false;  
  }
#if TOUCH_TYPE >= 2
  tp_group[TOUCH_BUTTON_2].tp = 1;
  if (StorageNvsE::instance().read("touch_1", &value_default)) {
    tp_group[TOUCH_BUTTON_2].state = 1;
    tp_group[TOUCH_BUTTON_2].value = value_default == 1 ? true : false;  
  }
#if TOUCH_TYPE >= 3
  tp_group[TOUCH_BUTTON_3].tp = 2;
  if (StorageNvsE::instance().read("touch_2", &value_default)) {
    tp_group[TOUCH_BUTTON_3].state = 1;
    tp_group[TOUCH_BUTTON_3].value = value_default == 1 ? true : false;  
  }
#if TOUCH_TYPE >= 4
  tp_group[TOUCH_BUTTON_4].tp = 3;
  if (StorageNvsE::instance().read("touch_3", &value_default)) {
    tp_group[TOUCH_BUTTON_4].state = 1;
    tp_group[TOUCH_BUTTON_4].value = value_default == 1 ? true : false;  
  }
#endif
#endif
#endif
#endif
  return true;
}


void IoTouchControllerTask::event(const EventPending& ev)
{
  disable_touch = ev.get_state();
}

void IoTouchControllerTask::event(const EnabledTouchDetail& ev)
{
  is_disable_touch.set(ev.get_id(), ev.get_state());
}

void IoTouchControllerTask::event(const SensitivityTouchValue& ev)
{
#if TOUCH_TYPE  >= 1
  tp_dev1_->set_threshold(TOUCH_BUTTON_MAX_CHANGE_RATE_1 * ev.get_value());
#if TOUCH_TYPE  >= 2
  tp_dev2_->set_threshold(TOUCH_BUTTON_MAX_CHANGE_RATE_2 * ev.get_value());
#if TOUCH_TYPE  >= 3
  tp_dev3_->set_threshold(TOUCH_BUTTON_MAX_CHANGE_RATE_3 * ev.get_value());
#if TOUCH_TYPE  >= 4
  tp_dev4_->set_threshold(TOUCH_BUTTON_MAX_CHANGE_RATE_4 * ev.get_value() * 1.1);
#endif
#endif
#endif
#endif
}

void IoTouchControllerTask::update_storage_cnt_relay(const char* storage_name)
{
  int32_t cnt_value = 0;
  if (StorageNvsE::instance().read(storage_name, &cnt_value)) {
    StorageNvsE::instance().write(storage_name, ++cnt_value);
  }
}

}