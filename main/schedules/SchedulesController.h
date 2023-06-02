#pragma once 

#include <memory>
#include <tuple>
#include <smooth/core/io/Input.h>
#include <smooth/core/io/Output.h>
#include <smooth/core/Application.h>
#include <smooth/core/timer/Timer.h>
#include <smooth/core/ipc/Publisher.h>
#include <smooth/core/timer/TimerExpiredEvent.h>
#include <smooth/core/ipc/SubscribingTaskEventQueue.h>

#include "common/digital/DigitalCountDown.h"
#include "common/digital/DigitalTimer.h"
#include "common/ObjectUpdateStatus.h"
#include "common/EventSleepLedRed.hpp"
#include "common/SyncTimerSer.h"
namespace iotTouch
{
class SchedulesController
    : public smooth::core::Task,
      public smooth::core::ipc::IEventListener<DigitalTimer>,
      public smooth::core::ipc::IEventListener<DigitalCountDown>,
      public smooth::core::ipc::IEventListener<SyncTimerSer>,
      public smooth::core::ipc::IEventListener<led::EventSleepLedRed>,
      public smooth::core::ipc::IEventListener<smooth::core::timer::TimerExpiredEvent>
{
  public:
    SchedulesController();

    void tick() override;

    void init() override;

    void event(const DigitalTimer& ev) override;
    
    void event(const DigitalCountDown& ev) override;

    void event(const smooth::core::timer::TimerExpiredEvent& event) override;

    void event(const SyncTimerSer& ev) override;

    void event(const led::EventSleepLedRed& ev) override;

    static void scheduleTimer(void *arg);

    static void scheduleCountDown(void *arg);

    static void scheduleSleepLedRed(void *arg);

  private:

    using DigitalTimerQueue = smooth::core::ipc::SubscribingTaskEventQueue<DigitalTimer>;
    std::shared_ptr<DigitalTimerQueue> timer_queue_;

    using DigitalCountDownQueue = smooth::core::ipc::SubscribingTaskEventQueue<DigitalCountDown>;
    std::shared_ptr<DigitalCountDownQueue> countdown_queue_;

    using SyncTimerSerQueue = smooth::core::ipc::SubscribingTaskEventQueue<SyncTimerSer>;
    std::shared_ptr<SyncTimerSerQueue> sync_timer_ser_queue_;

    using EventSleepLedRedQueue = smooth::core::ipc::SubscribingTaskEventQueue<led::EventSleepLedRed>;
    std::shared_ptr<EventSleepLedRedQueue> sleep_led_queue_;

    using ExpiredQueue = smooth::core::ipc::TaskEventQueue<smooth::core::timer::TimerExpiredEvent>;
    std::shared_ptr<ExpiredQueue> cron_queue_;
    smooth::core::timer::TimerOwner timer_;
    
    bool isSntp_ = false;

    static void turnOffLed(bool state);
    void executeTurnOffLed(const std::string start, const std::string end);
};
}