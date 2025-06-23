#pragma once 

#include <memory>
#include <smooth/core/Task.h>
#include <smooth/core/timer/Timer.h>
#include <smooth/core/Application.h>
#include <smooth/core/ipc/Publisher.h>
#include <smooth/core/util/json_util.h>
#include <smooth/core/timer/TimerExpiredEvent.h>
#include <smooth/core/ipc/SubscribingTaskEventQueue.h>

#include <smooth/core/filesystem/filesystem.h>
#include <smooth/core/filesystem/SPIFlash.h>

#include "common/SystemHelper.h"
#include "common/ObjectSetTimer.h"
#include "common/ObjectModeWifi.h"
#include "common/ObjectSetCountDown.h"
#include "common/ObjectSetDigitalState2Storage.h"
#include "common/ObjectEnableTouch.h"
#include "common/TriggerUpdateStorage.h"
#include "common/EventSleepLedRed.hpp"
#include "nvs/NvsEsp32.h"
#include "utils/Utils.h"

class Storage
    : public smooth::core::ipc::IEventListener<ObjectSetTimer>,
      public smooth::core::ipc::IEventListener<ObjectSetCountDown>,
      public smooth::core::ipc::IEventListener<ObjectSetDigitalState2Storage>,
      public smooth::core::ipc::IEventListener<ObjectModeWifi>,
      public smooth::core::ipc::IEventListener<TriggerUpdateStorage>,
      public smooth::core::ipc::IEventListener<led::EventSleepLedRed>
{
  public:
    Storage(std::string id, smooth::core::Task &task);

    void event(const ObjectSetTimer& ev) override;

    void event(const ObjectSetCountDown& ev) override;

    void event(const ObjectSetDigitalState2Storage& ev) override;
    
    void event(const TriggerUpdateStorage& ev) override;

    void event(const led::EventSleepLedRed& ev) override;

    void event(const ObjectModeWifi &mode);
  private:
    smooth::core::Task &task_;
    std::string id_;

    using ObjectSetTimerQueue = smooth::core::ipc::SubscribingTaskEventQueue<ObjectSetTimer>;
    std::shared_ptr<ObjectSetTimerQueue> set_timer_;

    using ObjectSetCountDownQueue = smooth::core::ipc::SubscribingTaskEventQueue<ObjectSetCountDown>;
    std::shared_ptr<ObjectSetCountDownQueue> set_countdown_;

    using ObjectSetDigitalStateQueue = smooth::core::ipc::SubscribingTaskEventQueue<ObjectSetDigitalState2Storage>;
    std::shared_ptr<ObjectSetDigitalStateQueue> digital_output_;

    using ModeConfigESPQueue = smooth::core::ipc::SubscribingTaskEventQueue<ObjectModeWifi>;
    std::shared_ptr<ModeConfigESPQueue> wifi_mode_;

    using TriggerUpdateStorageQueue = smooth::core::ipc::SubscribingTaskEventQueue<TriggerUpdateStorage>;
    std::shared_ptr<TriggerUpdateStorageQueue> update_;

    using EventSleepLedRedQueue = smooth::core::ipc::SubscribingTaskEventQueue<led::EventSleepLedRed>;
    std::shared_ptr<EventSleepLedRedQueue> event_sleep_led_;

    std::mutex guard_;
    void initHw();

    fireAlarm::system::SystemHelper helper_;
    smooth::core::filesystem::SPIFlash flash_ {
      smooth::core::filesystem::FlashMount::instance(),
      "app_storage",
      5,
      true
    };
    fireAlarm::Utils utils_;
};