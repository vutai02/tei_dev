
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
#include "common/digital/DigitalOutputValue.h"
#include "common/digital/DigitalIoLed.h"
#include "common/EventPairing.hpp"
#include "common/ConfigConstant.h"
#include "common/digital/Digital_OutPut_Alarm.h"
#include "common/digital/DigitalIoAlarm.h"
#include "common/ObjectControlOutputAlarm.h"
#include "common/IoInterface.h"
#include "input/input.h"
#include <vector>
#include "common/ObjectValueTimerNow.h"
#include "io/Button/button.h"
#include "common/ObjectTypeButton.h"
#include "common/ObjectDataSendToMetro.h"
#include "common/ConstantType.h"
#include "common/EventLed.hpp"
#include "common/digital/DigitalIoLed.h"
using namespace std;
using namespace std::chrono;
using namespace fireAlarm::common;
using namespace smooth::core::io;
using namespace smooth::core::timer;
using namespace smooth::core::logging;
using namespace smooth::core::json_util;
using namespace smooth::core::ipc;
using namespace smooth::core::io;
using namespace led;
namespace fireAlarm
{
  enum class ModeTouch
  {
    Smart = 0,
    Provision,
    Refactor,
    None
  };

  using ExpiredQueue = smooth::core::ipc::TaskEventQueue<smooth::core::timer::TimerExpiredEvent>;
  using EventLedQueue = smooth::core::ipc::SubscribingTaskEventQueue<DigitalIoLed>;
  // using DigitalOutputAlarmQueue = smooth::core::ipc::SubscribingTaskEventQueue<Digital_OutPut_Alarm>;
  using DigitalIoAlarmQueue = smooth::core::ipc::SubscribingTaskEventQueue<DigitalIoAlarm>;
  using DigitalIoLedQueue = smooth::core::ipc::SubscribingTaskEventQueue<DigitalIoLed>;
  class IoControllerTask
      : public smooth::core::Task,
        public smooth::core::ipc::IEventListener<DigitalIoLed>
  {
  public:
    IoControllerTask();
    void tick() override;
    void init() override;
   void event(const DigitalIoLed &event) override;
  private:
  InputAlarm input_alarm_1{IO_INPUT_ALARM_1, true, false, 5000, true};
  InputAlarm input_alarm_2{IO_INPUT_ALARM_2, true, false, 5000, true};
  Output out_put_alarm_1{IO_OUTPUT_ALARM_1, true, false, true};
  Output out_put_alarm_2{IO_OUTPUT_ALARM_2, true, false, true};
  Output out_put_alarm_3{IO_OUTPUT_ALARM_3, true, false, true};
  Button button_wifi{IO_CONFIG_WIFI, true, false};
  smooth::core::io::Output out_led_1{IO_LED1, true, false, true};
  smooth::core::io::Output out_led_2{IO_LED2, true, false, true};

  std::shared_ptr<DigitalIoLedQueue> DigitalIoLedQueue_;
 
  static void IOController_check_trigger_input(InputAlarm &input_);
  static void IOController_check_not_trigger(InputAlarm &input_);
  
  static void config_wifi_smart(Button &btn);
  static void config_wifi_manual(Button &btn);
  static void config_wifi_reset(Button &btn);
  
  static void led_smart_wifi(Button &btn);
  static void led_manual_wifi(Button &btn);
  static void led_reset_wifi(Button &btn);
  };
}