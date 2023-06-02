#pragma once 

#include <memory>
#include <tuple>
#include <smooth/core/Application.h>
#include <smooth/core/io/Output.h>
#include <smooth/core/io/Input.h>
#include <smooth/core/ipc/ISRTaskEventQueue.h>
#include <smooth/core/timer/Timer.h>
#include <smooth/core/timer/TimerExpiredEvent.h>
#include <smooth/core/ipc/SubscribingTaskEventQueue.h>
#include <smooth/core/ipc/Publisher.h>
#include <smooth/core/io/InterruptInputCB.h>
#include "common/digital/SensitivityTouchValue.h"
#include "common/digital/DigitalOutputValue.h"
#include "common/digital/DigitalTurnOffLedRed.h"
#include "common/digital/EnabledTouchDetail.h"
#include "common/digital/DigitalIoLed.h"
#include "touch_pad/iot_touchpad.h"
#include "common/EventPending.hpp"
#include "common/IoInterface.h"
#include "freertos/timers.h"

namespace iotTouch
{
  enum class ModeTouch
  {
    Smart = 0,
    Provision,
    Refactor,
    None
  };
  
  struct Message {
    bool is_start_cache;
    cache_io_relay_t data[TOUCH_TYPE];
  };

#define IO_TOUCH_ZERO                   GPIO_NUM_35
#define IO_RELAY_ALL                    GPIO_NUM_22
#define IO_LED                          GPIO_NUM_19

#define MAX_TIMER_TRIGGER_SMART         5
#define MAX_TIMER_TRIGGER_PROVISION     10  
#define MAX_TIMER_TRIGGER_REFACTOR      15  

#if TOUCH_TYPE  == 1

  #define IO_RELAY_1                    GPIO_NUM_17

  #define TOUCH_BUTTON_1                TOUCH_PAD_NUM3
#elif TOUCH_TYPE  == 2

  #define IO_RELAY_1                    GPIO_NUM_16
  #define IO_RELAY_2                    GPIO_NUM_25

  #define TOUCH_BUTTON_1                TOUCH_PAD_NUM2
  #define TOUCH_BUTTON_2                TOUCH_PAD_NUM4
#elif TOUCH_TYPE  == 3

  #define IO_RELAY_1                    GPIO_NUM_16
  #define IO_RELAY_2                    GPIO_NUM_17
  #define IO_RELAY_3                    GPIO_NUM_25

  #define TOUCH_BUTTON_1                TOUCH_PAD_NUM2
  #define TOUCH_BUTTON_2                TOUCH_PAD_NUM3
  #define TOUCH_BUTTON_3                TOUCH_PAD_NUM4
#else

  #define IO_RELAY_1                    GPIO_NUM_16
  #define IO_RELAY_2                    GPIO_NUM_17
  #define IO_RELAY_3                    GPIO_NUM_25
  #define IO_RELAY_4                    GPIO_NUM_26

  #define TOUCH_BUTTON_1                TOUCH_PAD_NUM2
  #define TOUCH_BUTTON_2                TOUCH_PAD_NUM3
  #define TOUCH_BUTTON_3                TOUCH_PAD_NUM4
  #define TOUCH_BUTTON_4                TOUCH_PAD_NUM0
#endif


class IoTouchControllerTask
    : public smooth::core::Task,
      public smooth::core::ipc::IEventListener<DigitalOutputValue>,
      public smooth::core::ipc::IEventListener<DigitalIoLed>,
      public smooth::core::ipc::IEventListener<DigitalTurnOffLedRed>,
      public smooth::core::ipc::IEventListener<EventPending>,
      public smooth::core::ipc::IEventListener<EnabledTouchDetail>,
      public smooth::core::ipc::IEventListener<SensitivityTouchValue>
      // public smooth::core::ipc::IEventListener<smooth::core::io::InterruptInputEvent>
{
  public:
    IoTouchControllerTask();

    void tick() override;

    void init() override;

    void event(const DigitalOutputValue& ev) override;

    void event(const DigitalIoLed& ev) override;
    
    void event(const DigitalTurnOffLedRed& ev) override;

    void event(const EventPending& ev) override;

    void event(const EnabledTouchDetail& ev) override;

    void event(const SensitivityTouchValue& ev) override;

    void set_io(uint8_t position, bool value);
  
  private:

#if TOUCH_TYPE >= 1
    smooth::core::io::Output out_relay_1_{IO_RELAY_1, true, false, true};
#if TOUCH_TYPE >= 2
    smooth::core::io::Output out_relay_2_{IO_RELAY_2, true, false, true};
#if TOUCH_TYPE >= 3
    smooth::core::io::Output out_relay_3_{IO_RELAY_3, true, false, true};
#if TOUCH_TYPE >= 4
    smooth::core::io::Output out_relay_4_{IO_RELAY_4, true, false, true};
#endif
#endif
#endif
#endif
    smooth::core::io::Output out_relay_{IO_RELAY_ALL, true, false, true};
    smooth::core::io::Output out_led_{IO_LED, true, false, true};


#if TOUCH_TYPE  >= 1
    CTouchPad* tp_dev1_;
#if TOUCH_TYPE  >= 2
    CTouchPad* tp_dev2_;
#if TOUCH_TYPE  >= 3
    CTouchPad* tp_dev3_;
#if TOUCH_TYPE  >= 4
    CTouchPad* tp_dev4_;
#endif
#endif
#endif
#endif
    
    using DigitalOutputValueQueue = smooth::core::ipc::SubscribingTaskEventQueue<DigitalOutputValue>;
    std::shared_ptr<DigitalOutputValueQueue> relay_value_;

    using DigitalIoLedQueue = smooth::core::ipc::SubscribingTaskEventQueue<DigitalIoLed>;
    std::shared_ptr<DigitalIoLedQueue> led_;

    using DigitalTurnOffLedQueue = smooth::core::ipc::SubscribingTaskEventQueue<DigitalTurnOffLedRed>;
    std::shared_ptr<DigitalTurnOffLedQueue> turn_off_led_;

    using EventPendingQueue = smooth::core::ipc::SubscribingTaskEventQueue<EventPending>;
    std::shared_ptr<EventPendingQueue> ota_pending_touch_;

    using EnabledTouchDetailQueue = smooth::core::ipc::SubscribingTaskEventQueue<EnabledTouchDetail>;
    std::shared_ptr<EnabledTouchDetailQueue> state_touch_detail_;

    using SensitivityTouchValueQueue = smooth::core::ipc::SubscribingTaskEventQueue<SensitivityTouchValue>;
    std::shared_ptr<SensitivityTouchValueQueue> sensitivity_touch_;

    smooth::core::io::InterruptInputCB zero_detect_;

    bool prepare_hw();
    void initialize_touch();
    static void isr_task(void* context);
    void isr_process(struct Message& msg);
    static void isr_zero_detect(void* context);

    void update_storage_cnt_relay(const char* storage_name);
};
}