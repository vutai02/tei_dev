#include <chrono>
#include <smooth/core/logging/log.h>
#include <smooth/core/ipc/Publisher.h>
#include <smooth/core/json/JsonFile.h>
#include <smooth/core/task_priorities.h>
#include "Storage.h"
#include "utils/timer_id.h"
#include "utils/DataCache.hpp"
#include "StorageWifi.h"
#include "StorageIoTouch.h"
#include "StorageSchedule.h"
#include "StorageInfoDevice.h"
#include "common/ObjectModeWifi.h"
#include "common/ConfigConstant.h"
#include "common/digital/DigitalTimer.h"
#include "common/digital/DigitalCountDown.h"
#include "common/digital/EnabledTouchDetail.h"
#include "common/EventLed.hpp"
#include "nvs/StorageNvsE.h"

using namespace iotTouch::storage;

using namespace iotTouch::common;
using namespace iotTouch::system;
using namespace smooth::core;
using namespace smooth::core::timer;
using namespace smooth::core::ipc;
using namespace smooth::core::json;
using namespace smooth::core::json_util;
using namespace smooth::core::filesystem;
using namespace std::chrono;


#define ID_ON_OFF_ALL_TOUCH     4

Storage::Storage(std::string id, smooth::core::Task &task)
  :task_(task),
  id_(std::move(id)),
  set_timer_(ObjectSetTimerQueue::create(2, task_, *this)),
  set_countdown_(ObjectSetCountDownQueue::create(2, task_, *this)),
  digital_output_(ObjectSetDigitalStateQueue::create(4, task_, *this)),
  wifi_mode_(ModeConfigESPQueue::create(2, task_, *this)),
  update_(TriggerUpdateStorageQueue::create(2, task_, *this)),
  event_sleep_led_(EventSleepLedRedQueue::create(2, task_, *this))
{
  initHw();
}

void Storage::initHw()
{
  // init storage wifi.
  if (flash_.mount()) {

    // StorageWifi::instance().load();
    StorageSchedule::instance().load();
    StorageInfoDevice::instance().load();
    
    // if (!StorageWifi::instance().isStorageWifiValid()) {
    //   StorageWifi::instance().write_default();
    //   StorageWifi::instance().load();
    // }
    if (!StorageSchedule::instance().isStorageScheduleValid()) {
      StorageSchedule::instance().write_default();
      StorageSchedule::instance().load();
    }

    if (!StorageInfoDevice::instance().isStorageInfoDeviceValid()) {
      StorageInfoDevice::instance().write_default();
      StorageInfoDevice::instance().load();
    }

    StorageNvsE::instance().initialize();
    iotTouch::DataCache::instance();

    // Log::info(id_, "StorageWifi {}", StorageWifi::instance().get().dump());
    Log::info(id_, "StorageSchedule {}", StorageSchedule::instance().get().dump());
    Log::info(id_, "StorageInfoDevice {}", StorageInfoDevice::instance().get().dump());
  }
  flash_.unmount();
}

void Storage::event(const ObjectSetTimer& ev)
{
  Log::info(id_, "setTime id_: {0} state: {1} cron: {2}", ev.get_id(), 
        ev.get_state(), ev.get_cron());
  // TODO save to database
  if (flash_.mount()) {
    try {
        auto& schedules = StorageSchedule::instance().get()[SCHEDULES];
        std::string action = ev.get_action();
        utils_.tolower(action);

        for (const auto& item : schedules.items()) {
          if (std::to_string(ev.get_id()) == item.key()) {
            
            auto jsonArray = nlohmann::json::array();
            auto object = nlohmann::json::object();

            object[INDEX] = ev.get_index();
            object[STATE] = ev.get_state();
            object[TIMER] = ev.get_cron();
            object[REPEAT] = ev.get_repeat();

            if (action == "insert") {
              bool is_update = false;
              for (const auto& itTimers : item.value()[TIMER].items()) {
                if (ev.get_index() == itTimers.value()[INDEX].get<int>()) {
                  item.value()[TIMER].at(ev.get_index()) = object;
                  is_update = true;
                }
              }
              if (!is_update)
                item.value()[TIMER].push_back(object);

              Publisher<DigitalTimer>::publish(DigitalTimer(
                ev.get_id(),
                ev.get_state(),
                ev.get_repeat(),
                ev.get_index(),
                ev.get_cron()
              ));

              break;
            }
            else if (action == "delete") {

              for (const auto& itTimers : item.value()[TIMER].items()) {
                if (ev.get_index() != itTimers.value()[INDEX].get<int>()) {
                  jsonArray.push_back(itTimers.value());
                }
              }
              
              item.value()[TIMER] = jsonArray; 

              Publisher<DigitalTimer>::publish(DigitalTimer(
                ev.get_id(),
                ev.get_state(),
                ev.get_repeat(),
                ev.get_index(),
                ev.get_cron(),
                "Delete"
              ));
              
              break;
            }
            break;
          }
        }
        
        // Log::info(id_, "schedules {}", schedules.dump());

        StorageSchedule::instance().save();
    }
    catch(const std::exception& e) {
      std::cerr << e.what() << '\n';
    }
  }
  flash_.unmount();
}

void Storage::event(const led::EventSleepLedRed& ev)
{
  if (flash_.mount()) {    
    try {
      StorageSchedule::instance().get()["sleepRed"]["start"] = ev.get_start();
      StorageSchedule::instance().get()["sleepRed"]["end"] = ev.get_end();
      StorageSchedule::instance().save();
    }
    catch(const std::exception& e) {
      std::cerr << e.what() << '\n';
    }
  }
  flash_.unmount();
}

void Storage::event(const ObjectSetCountDown& ev)
{
  if (flash_.mount()) {
    try {
      Log::info(id_, "setCountDown id_: {0} state: {1} cron: {2} action: {3}",
        ev.get_id(), ev.get_state(), ev.get_cron(), ev.get_action());
      
      std::string action = ev.get_action();
      utils_.tolower(action);
      
      // TODO save to database
      bool enable = true;
      if (action == "delete") {
        enable = false;
      }

      Publisher<DigitalCountDown>::publish(
        DigitalCountDown(
          ev.get_id(),
          ev.get_state(),
          ev.get_cron(),
          enable? "insert": "delete"
        )
      );

      auto& schedules = StorageSchedule::instance().get()[SCHEDULES];

      for (const auto& item : schedules.items()) {
        if (std::to_string(ev.get_id()) == item.key()) {
          item.value()[COUNTDOWN][ENABLE] = enable;
          item.value()[COUNTDOWN][STATE] = ev.get_state();
          item.value()[COUNTDOWN][TIMER] = ev.get_cron();
        }
      }
      
      // Log::info(id_, "schedules {}", StorageSchedule::instance().get().dump());
      StorageSchedule::instance().save();
    }
    catch(const std::exception& e) {
      std::cerr << e.what() << '\n';
    }
  }
  flash_.unmount();
}

void Storage::event(const ObjectSetDigitalState2Storage& ev)
{
  Log::info(id_, "cached id_: {0} state: {1}", ev.get_io(), ev.get_state());
  std::lock_guard<std::mutex> lock(guard_);
  
  int32_t touch_value = ev.get_state() ? 1: 0;
  StorageNvsE::instance().write("id_syc", ev.get_id());

  if (ev.get_io() >= ID_ON_OFF_ALL_TOUCH) {
    int32_t cnt_value = 0;
#if TOUCH_TYPE  >= 1
    StorageNvsE::instance().write("touch_0", touch_value);
#if TOUCH_TYPE  >= 2
    StorageNvsE::instance().write("touch_1", touch_value);
#if TOUCH_TYPE  >= 3
    StorageNvsE::instance().write("touch_2", touch_value);
#if TOUCH_TYPE  >= 4
    StorageNvsE::instance().write("touch_3", touch_value);
#endif
#endif
#endif
#endif
  }
  else {
    std::string key = "touch_" + std::to_string(ev.get_io());
    StorageNvsE::instance().write(key.c_str(), touch_value);
  }
}

void Storage::event(const ObjectModeWifi &mode)
{
  switch (mode.get_mode())
  {
  case WifiMode::ap_mode:
    StorageNvsE::instance().write(WIFI_MODE, WifiMode::ap_mode);
    break;
  case WifiMode::smartconfig_mode:
    StorageNvsE::instance().write(WIFI_MODE, WifiMode::smartconfig_mode);
    break;
  case WifiMode::provisioning_mode:
    StorageNvsE::instance().write(WIFI_MODE, WifiMode::provisioning_mode);
    break;
  case WifiMode::reset_mode:
    { 
      if (flash_.mount()) {
        unlink(StorageSchedule::instance().get_path());
        unlink(StorageInfoDevice::instance().get_path());

        int32_t cnt_t0_value = 0;
        int32_t cnt_t1_value = 0;
        int32_t cnt_t2_value = 0;
        int32_t cnt_t3_value = 0;
    #if TOUCH_TYPE  >= 1
        StorageNvsE::instance().read("cnt_touch_0", &cnt_t0_value);
    #if TOUCH_TYPE  >= 2
        StorageNvsE::instance().read("cnt_touch_1", &cnt_t1_value);
    #if TOUCH_TYPE  >= 3
        StorageNvsE::instance().read("cnt_touch_2", &cnt_t2_value);
    #if TOUCH_TYPE  >= 4
        StorageNvsE::instance().read("cnt_touch_3", &cnt_t3_value);
    #endif
    #endif
    #endif
    #endif
    
        StorageNvsE::instance().erase_all();

    #if TOUCH_TYPE  >= 1
        StorageNvsE::instance().write("cnt_touch_0", cnt_t0_value);
    #if TOUCH_TYPE  >= 2
        StorageNvsE::instance().write("cnt_touch_1", ++cnt_t1_value);
    #if TOUCH_TYPE  >= 3
        StorageNvsE::instance().write("cnt_touch_2", ++cnt_t2_value);
    #if TOUCH_TYPE  >= 4
        StorageNvsE::instance().write("cnt_touch_3", ++cnt_t3_value);
    #endif
    #endif
    #endif
    #endif
        flash_.unmount();
        helper_.restartChip();
      }
      else {
        flash_.unmount();
        Publisher<led::EventLed>::publish(
          led::EventLed("exit")
        );
      }
    }
    break;
  default:
    break;
  }
}

void Storage::event(const TriggerUpdateStorage &ev)
{
  if (flash_.mount()) {
    Log::info(id_, "TriggerUpdateStorage: {}", ev.get_trigger());

    auto& configuration = StorageInfoDevice::instance().get()[CONFIGURATION];
    auto& config_mqtt = configuration[MQTT];
    auto& topic = configuration[TOPIC];

    config_mqtt[USER] = iotTouch::DataCache::instance().get(USER);
    config_mqtt[PASS] = iotTouch::DataCache::instance().get(PASS);
    config_mqtt[CLIENT_ID] = iotTouch::DataCache::instance().get(CLIENT_ID);
    config_mqtt[MQTT_ENDPOINT] = iotTouch::DataCache::instance().get(MQTT_ENDPOINT);
    config_mqtt[MQTT_PORT] = stoi(iotTouch::DataCache::instance().get(MQTT_PORT));
    config_mqtt[SCHEME] = iotTouch::DataCache::instance().get(SCHEME);
    
    configuration[ACTIVATE] = iotTouch::DataCache::instance().get(ACTIVATE) == "1" ? true: false;
    configuration[DEVICE][ID] = iotTouch::DataCache::instance().get(EXTERNAL_ID);
    configuration[DEVICE][TOKEN] = iotTouch::DataCache::instance().get(EXTERNAL_KEY);
    
    topic["0"][NAME] = "config";
    topic["0"][ID] = iotTouch::DataCache::instance().get("config");

    topic["1"][NAME] = "control";
    topic["1"][ID] = iotTouch::DataCache::instance().get("control");
    
    topic["2"][NAME] = "notify";
    topic["2"][ID] = iotTouch::DataCache::instance().get("notify");

    StorageInfoDevice::instance().save();
    
    std::this_thread::sleep_for(std::chrono::milliseconds{ 40 });

    // update wifi
    StorageNvsE::instance().write(SSID, iotTouch::DataCache::instance().get(SSID));
    StorageNvsE::instance().write(KEY, iotTouch::DataCache::instance().get(KEY));
    StorageNvsE::instance().write(WIFI_MODE, WifiMode::ap_mode);

    StorageWifi::instance().save();
  }
  flash_.unmount();

  // helper_.restartChip();
}