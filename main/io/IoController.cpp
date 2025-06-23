#include <memory>
#include <driver/gpio.h>
#include <smooth/core/util/ByteSet.h>
#include <smooth/core/ipc/Publisher.h>
#include <smooth/core/util/json_util.h>
#include <smooth/core/task_priorities.h>
#include "utils/timer_id.h"
#include "utils/DataCache.hpp"
#include "common/EventLed.hpp"
#include "common/EventOutputAlarm.hpp"
#include "common/IoInterface.h"
#include "IoController.h"
#include "common/ConstantType.h"
#include "common/ConfigConstant.h"
#include "storage/nvs/StorageNvsE.h"
#include "common/ObjectTypeButton.h"
#include "common/digital/DigitalInputValue.h"
#include "common/digital/DigitalOutputValue.h"
#include "common/EventRF433Pairing.hpp"
#include "common/ObjectDataSendToMetro.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include <string>
#include "storage/StorageLevelInput.h"

namespace fireAlarm
{
  static constexpr const char *IO_TAG = "IO_tag";

  int32_t timer_trigger_fire = 0;
  int32_t timer_output = 0;
  int32_t timer_feedback = 0;

  IoControllerTask::IoControllerTask()
      : Task(IO_TAG, 4 * 1024, 6, milliseconds{500}),
        // DigitalIoAlarmQueue_(DigitalIoAlarmQueue::create(10, *this, *this)),
        DigitalIoLedQueue_(DigitalIoLedQueue::create(4, *this, *this))
  {
    button_wifi.setManual(config_wifi_manual);
    button_wifi.setSmart(config_wifi_smart);
    button_wifi.setReset(config_wifi_reset);

    button_wifi.setSmartLed(led_smart_wifi);
    button_wifi.setManualLed(led_manual_wifi);
    button_wifi.setResetLed(led_reset_wifi);

    input_alarm_1.check_trigger_input(IOController_check_trigger_input);
    input_alarm_2.check_trigger_input(IOController_check_trigger_input);
    input_alarm_1.check_not_trigger(IOController_check_not_trigger);
    input_alarm_2.check_not_trigger(IOController_check_not_trigger);
   
  }

  void IoControllerTask::tick()
  {
      button_wifi.tick();
      // out_put_alarm_2.set(true);
      // out_put_alarm_3.set(true);
      // Log::info(IO_TAG,"BAT");
      // vTaskDelay(pdMS_TO_TICKS(1000));

      // out_put_alarm_2.set(false);
      // out_put_alarm_3.set(false);
      // Log::info(IO_TAG,"TAT");
  }
  
  void IoControllerTask::init()
  {
    Log::info(IO_TAG,"init io");
    // out_put_alarm_1.set(true);
    out_put_alarm_2.set(true);
    out_put_alarm_3.set(true);
    out_put_alarm_1.set(false); // buzer

    out_led_1.set(true);
    out_led_2.set(true);

    // out_put_alarm_1(true);
    // out_put_alarm_2(true);
    // out_put_alarm_3(true);
  /* button new */
  }
  
  void IoControllerTask::IOController_check_trigger_input(InputAlarm &input)
  {
    Log::info(IO_TAG,"input :{}", input.state);
  }

  void IoControllerTask::IOController_check_not_trigger(InputAlarm &input)
  {
    Log::info(IO_TAG,"input:{}", input.state);
  }

  // void IoControllerTask::event(const Digital_OutPut_ADilarm &ev)
  // {
  //   out_led_1.set(ev.get_status());
  //   Log::info(IO_TAG, "state:{}", ev.get_status());
  //   // vTaskDelay()
  //   out_led_2.set(ev.get_satus());
  //   Log::info(IO_TAG, "State: ", ev.get_status());
  // }

  // void IoControllerTask::event(const Digital_OutPut_Alarm &event)
  // {
  //   out_put_alarm_2.set(event.get_status());
  //   Log::info(IO_TAG, "state:{}", event.get_status());
  // }

  void IoControllerTask::event(const DigitalIoLed &ev)
  {
    out_led_1.set(ev.get_state());
    Log::info(IO_TAG, "out put led:{}", ev.get_state());
    // out_led_2.set(ev.get_state());
  }

  void IoControllerTask::config_wifi_smart(Button &btn) // publish to metro
  {
    if (fireAlarm::DataCache::instance().get(ACTIVATE) == "0")
    {
      Log::info(IO_TAG, "wifi_smart");
      Publisher<ObjectTypeButton>::publish(ObjectTypeButton(TypeButton::SMART)); // 0
    }
  }
  void IoControllerTask::config_wifi_manual(Button &btn) // publish to metro
  {
    if (fireAlarm::DataCache::instance().get(ACTIVATE) == "0")
    {
      Log::info(IO_TAG, "wifi_manual");
      Publisher<ObjectTypeButton>::publish(ObjectTypeButton(TypeButton::PROVISIONING)); // 0
    }
  }

  void IoControllerTask::config_wifi_reset(Button &btn) // publish to metro
  {
    Log::info(IO_TAG, "wifi_reset");
    Publisher<ObjectTypeButton>::publish(ObjectTypeButton(TypeButton::REFACTORY_SYSTEM)); // 0
  }

  void IoControllerTask::led_smart_wifi(Button &btn) // publish to metro
  {
    if (fireAlarm::DataCache::instance().get(ACTIVATE) == "0")
    {
      Log::info(IO_TAG, "led_smart");
      Publisher<led::EventLed>::publish(
         led::EventLed("smart"));
    }
  }
  void IoControllerTask::led_manual_wifi(Button &btn) // publish to metro
  {
    if (fireAlarm::DataCache::instance().get(ACTIVATE) == "0")
    {
      Log::info(IO_TAG, "led_provisoning");
      Publisher<led::EventLed>::publish(
         led::EventLed("provisioning"));
    }
  }
  void IoControllerTask::led_reset_wifi(Button &btn) // pidfublish to metro
  {
    Log::info(IO_TAG, "led_refactor");
    Publisher<led::EventLed>::publish(
       led::EventLed("refactor"));
  }
}  

