
#include <memory>
#include <nlohmann/json.hpp>
#include <smooth/core/task_priorities.h>
#include <smooth/core/ipc/Publisher.h>
#include <smooth/core/util/ByteSet.h>
#include "common/ObjectDataSendToMetro.h"
#include "common/digital/DigitalTurnOffLedRed.h"
#include "storage/StorageSchedule.h"
#include "common/ConfigConstant.h"
#include "SchedulesController.h"
#include "common/ConstantType.h"
#include "common/IoInterface.h"
#include "CronAlarms.h"
#include "utils/timer_id.h"

using namespace std;
using namespace std::chrono;
using namespace iotTouch::common;
using namespace smooth::core::io;
using namespace smooth::core::timer;
using namespace smooth::core::logging;
using namespace smooth::core::util;
using namespace smooth::core::ipc;
using namespace iotTouch::storage;

namespace iotTouch
{

CronId id;
CronId id_start_timer;
CronId id_end_timer;
iot_tp_dev_t tp_timer;
iot_tp_dev_t tp_countdown;
static smooth::core::util::ByteSet is_reset(0);
static std::map<int, int> map_timer{};
static std::map<int, int> map_countdown{};
static constexpr const char* Sch_TAG = "Schedules";

SchedulesController::SchedulesController()
    : Task(Sch_TAG, 3 * 1024, smooth::core::APPLICATION_BASE_PRIO, seconds{10}),
    timer_queue_(DigitalTimerQueue::create(2, *this, *this)),
    countdown_queue_(DigitalCountDownQueue::create(2, *this, *this)),
    sync_timer_ser_queue_(SyncTimerSerQueue::create(2, *this, *this)),
    sleep_led_queue_(EventSleepLedRedQueue::create(2, *this, *this)),
    cron_queue_(ExpiredQueue::create(2, *this, *this))
{
}

void SchedulesController::tick()
{
  // Log::info("ioTouch", "tick");
}

void SchedulesController::scheduleTimer(void *arg)
{
  auto io = static_cast<iot_tp_dev_t*>(arg);
  if (io != nullptr) {
    Log::info(Sch_TAG, "Timer relay: {0}: state: {1} id {2}",
      (int)io->tp, (bool)io->value, (int)io->id);
    
    nlohmann::json payload;
    payload["touch"] = io->tp; 
    payload["state"] = io->value;

    Publisher<ObjectDataSendToMetro>::publish(ObjectDataSendToMetro(
      TYPE_INTERNAL_SET_SCHEDULE,
      "update",
      payload
    ));
  }
  time_t now{};
  tm timeinfo{};
  time(&now);
  localtime_r(&now, &timeinfo);
  Log::info(Sch_TAG, "Sntp {}", asctime(&timeinfo));
}

void SchedulesController::scheduleCountDown(void *arg)
{
  iot_tp_dev_t* io = (iot_tp_dev_t*) arg;
  if (io != nullptr) {
    Log::debug(Sch_TAG, "Countdown, relay: {0}: state: {1} id {2}",
      (int)io->tp, (bool)io->value, (int)io->id);
    nlohmann::json payload;
    payload["touch"] = io->tp; 
    payload["state"] = io->value;

    Publisher<ObjectDataSendToMetro>::publish(ObjectDataSendToMetro(
      TYPE_INTERNAL_SET_SCHEDULE,
      "update",
      payload
    ));
  }
  time_t now{};
  tm timeinfo{};
  time(&now);
  localtime_r(&now, &timeinfo);
  Log::debug(Sch_TAG, "Sntp now: {}", asctime(&timeinfo));
}

void SchedulesController::scheduleSleepLedRed(void *arg)
{
  iot_tp_dev_t* io = (iot_tp_dev_t*) arg;
  Log::debug(Sch_TAG, "scheduleSleepLedRed state: {0} disabled: {1}",(bool)io->value, (bool)io->value);
  turnOffLed((bool)io->value);
}

void SchedulesController::init()
{
  esp_log_level_set(
    Sch_TAG,
    static_cast<esp_log_level_t>(TOUCH_SCHEDULES_LOGGING_LEVEL)
  );
}

void SchedulesController::event(const SyncTimerSer& ev)
{
  if (timer_) {
    timer_->stop();
  }

  timer_ = Timer::create(SCHEDULES_TICK, cron_queue_, true, std::chrono::seconds{1});
  timer_->start();

  if (!isSntp_) {
    isSntp_ = true;
    // TODO cache countdown
    auto& schedules = StorageSchedule::instance().get()[SCHEDULES];

    for (const auto& item : schedules.items()) {
      // Check countdown 
      if (item.value()[COUNTDOWN][ENABLE].get<bool>()) {
        tp_countdown.id = 1;
        tp_countdown.tp = stoi(item.key().c_str());
        tp_countdown.value = item.value()[COUNTDOWN][STATE].get<bool>();
        std::string cron = item.value()[COUNTDOWN][TIMER].get<std::string>();
        Cron.create(
          const_cast<char *>(cron.c_str()),
          scheduleCountDown, 
          tp_countdown, 
          true
        ); 
      }
      for (const auto& itTimer : item.value()[TIMER].items()) {
            
        tp_timer.id = 1;
        tp_timer.tp = stoi(item.key().c_str());
        tp_timer.value = itTimer.value()[STATE].get<bool>();
        bool repeat = itTimer.value()[REPEAT].get<bool>();
        std::string cron = itTimer.value()[TIMER].get<std::string>();
        id = Cron.create(
          const_cast<char *>(cron.c_str()),
          scheduleTimer, tp_timer, !repeat
        );

        map_timer[itTimer.value()[INDEX].get<int>()] = id;
      }
    }

    auto turn_off_led = StorageSchedule::instance().get()["sleepRed"];

    if (turn_off_led.contains("start") && turn_off_led.contains("end")) {
      executeTurnOffLed(turn_off_led["start"], turn_off_led["end"]);
    }
  }
}

void SchedulesController::event(const DigitalTimer& ev)
{
  Log::debug(Sch_TAG, "timer id:{0} state: {1} cron: {2}", 
    ev.get_input(), ev.get_value(), ev.get_cron());

  if (ev.get_action() == "delete") {
    for (auto& i: map_timer) {
      if (i.first == ev.get_input()) {
        Cron.free(i.second);
        map_timer.erase(i.first);
      }
    }
  }
  else {
    tp_timer.id = 1;
    tp_timer.tp = ev.get_input();
    tp_timer.value = ev.get_value();

    id = Cron.create(
      const_cast<char *>(ev.get_cron().c_str()),
      scheduleTimer,
      tp_timer,
      !ev.get_repeat()
    );

    if (id != dtINVALID_ALARM_ID)
      map_timer[ev.get_input()] = id;
  }
}

void SchedulesController::event(const DigitalCountDown& ev)
{
  Log::debug(Sch_TAG, "countdown id:{0} state: {1} cron: {2}", ev.get_id(), ev.get_state(), ev.get_cron());
  
  if (ev.get_action() == "delete") {
    for (auto& i: map_countdown) {
      if (i.first == ev.get_id()) {
        Cron.free(i.second);
        map_countdown.erase(i.first);
      }
    }
  }
  else {
    tp_countdown.id = 1;
    tp_countdown.tp = ev.get_id();
    tp_countdown.value = ev.get_state();

    // check id created?
    for (auto& i: map_countdown) {
      if (i.first == ev.get_id()) {
        Cron.free(i.second);
        map_countdown.erase(i.first);
      }
    }

    id = Cron.create(
      const_cast<char *>(ev.get_cron().c_str()),
      scheduleCountDown,
      tp_countdown,
      true
    );
    
    if (id != dtINVALID_ALARM_ID)
      map_countdown[ev.get_id()] = id;
  }
}

void SchedulesController::event(const led::EventSleepLedRed& ev)
{
  executeTurnOffLed(ev.get_start(), ev.get_end());
}

void SchedulesController::event(const smooth::core::timer::TimerExpiredEvent& event)
{
  if(event.get_id() == SCHEDULES_TICK) {
    Cron.execute();
  }
}

void SchedulesController::turnOffLed(bool state)
{
  Publisher<DigitalTurnOffLedRed>::publish(DigitalTurnOffLedRed(state));
}

void SchedulesController::executeTurnOffLed(const std::string start, const std::string end)
{
  tm timenow_tm{};
  auto timenow = system_clock::to_time_t(system_clock::now());
  localtime_r(&timenow, &timenow_tm);
  int t_n = timenow_tm.tm_hour * 100 + timenow_tm.tm_min;
  int t_start = 0;
  int t_end = 0;

  if (start.length()) {
    tp_timer.id = 1;
    tp_timer.tp = 5;
    tp_timer.value = true;
    
    Cron.free(id_start_timer);

    id_start_timer = Cron.create(
      const_cast<char *>(start.c_str()),
      scheduleSleepLedRed, tp_timer, false
    );

    if (id_start_timer != dtINVALID_ALARM_ID) {
      time_t start_time = Cron.getPrevTrigger(id_start_timer);
      tm start_time_tm{};
      localtime_r(&start_time, &start_time_tm);

      t_start = start_time_tm.tm_hour * 100 + start_time_tm.tm_min;
      Log::info(Sch_TAG, "---->>> Now: {0} start: {1}", t_n, t_start);

      if (t_n >= t_start) {
        // den tat
        turnOffLed(true);
      }

      if (t_n < t_start) {
        // den sang
        turnOffLed(false);
      }
    }
  }
  else {
    turnOffLed(false);
    Cron.free(id_start_timer);
  }
  
  if (end.length()) {
    tp_timer.id = 1;
    tp_timer.tp = 5;
    tp_timer.value = false;
    
    Cron.free(id_end_timer);

    id_end_timer = Cron.create(
      const_cast<char *>(end.c_str()),
      scheduleSleepLedRed, tp_timer, false
    );
    if (id_end_timer != dtINVALID_ALARM_ID) {
      time_t end_time = Cron.getPrevTrigger(id_end_timer);
      tm end_time_tm{};
      localtime_r(&end_time, &end_time_tm);

      t_end = end_time_tm.tm_hour * 100 + end_time_tm.tm_min;
      Log::info(Sch_TAG, "--->> Now: {0} end: {1}", t_n, t_end);

      if (t_n < t_end) {
        if (t_n >= t_start) {
          // den tat
          turnOffLed(true);
        }
        else {
          // den sang
          turnOffLed(false);
        }
      }

      if (t_n >= t_end) {
        // den sang
        if (t_n >= t_start) {
          // den tat
          turnOffLed(true);
        }
        else {
          // den sang
          turnOffLed(false);
        }
      }
    }
  }
  else {
    turnOffLed(false);
    Cron.free(id_end_timer);
  }
}

}
