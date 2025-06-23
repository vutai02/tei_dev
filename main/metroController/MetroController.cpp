
#include <memory>
#include <iostream>
#include <smooth/core/util/ByteSet.h>
#include <smooth/core/ipc/Publisher.h>
#include <smooth/core/task_priorities.h>
#include <smooth/core/util/json_util.h>
#include <nlohmann/json.hpp>
#include <esp_timer.h>
#include "utils/timer_id.h"
#include "utils/DataCache.hpp"
#include "MetroController.h"
#include "common/ObjectOta.h"
#include "common/ConstantType.h"
#include "common/ConfigConstant.h"
#include "common/ObjectModeWifi.h"
#include "common/ObjectModeConfigESP.h"
#include "common/ObjectDataDev2Ser.h"
#include "common/digital/DigitalOutputValue.h"
#include "common/ObjectSetDigitalState2Storage.h"
#include "common/digital/EnabledTouchDetail.h"
#include "common/ObjectPingServer.h"
#include "common/ObjectEnableTouch.h"
#include "common/ObjectSetTimer.h"
#include "common/EventLed.hpp"
#include "common/ObjectSetCountDown.h"
#include "storage/StorageSchedule.h"
#include "storage/nvs/StorageNvsE.h"
#include "common/EventSleepLedRed.hpp"


using namespace std;
using namespace std::chrono;
using namespace fireAlarm::common;
using namespace smooth::core::io;
using namespace smooth::core::timer;
using namespace smooth::core::logging;
using namespace smooth::core::json_util;
using namespace smooth::core::ipc;

namespace fireAlarm
{
static constexpr const char* METRO_TAG = "Metro_tag";
static constexpr std::chrono::milliseconds interval(1000);
constexpr const int MAX_TIMER_UPDATE_PING = 30; // seconds
constexpr const int TIMER_15_MINUTES = 900; // seconds

#define DEBOUNCE_PUSH_STATE std::chrono::milliseconds(100)

MetroController::MetroController()
    : Task(METRO_TAG, 6 * 1024, smooth::core::APPLICATION_BASE_PRIO, milliseconds{1000}),
    data_to_metro_(DatatoMetroQueue::create(4, *this, *this)),
    digital_input_(DigitalInputValueQueue::create(2, *this, *this)),
    type_button_(ObjectTypeButtonQueue::create(2, *this, *this)),
    ping_ser_(ObjectMetroPingServerQueue::create(3, *this, *this)),
    timer_queue_(ExpiredQueue::create(4, *this, *this)),
    timer_(0, timer_queue_, true, interval)
{
  timer_->start();
}

void MetroController::tick()
{
  // Log::info(METRO_TAG, "tick");
}

void MetroController::init()
{
  esp_log_level_set(METRO_TAG, static_cast<esp_log_level_t>(TOUCH_METRO_LOGGING_LEVEL));
  // Log::info(METRO_TAG, "init MetroController");
  for (int i = 0; i < TOUCH_TYPE; i++) {
    cacheState[i].is_push = false;
  }
}

// Send event from server to metro
void MetroController::event(const ObjectDataSendToMetro& event)
{
  Log::info(METRO_TAG, "type: {0} \n \
                        action: {1} \n\
                        payload: {2}", event.get_type(), event.get_action(), event.get_json().dump());
  try{
    std::string action = event.get_action();
    utils_.tolower(action);

    nlohmann::json data = event.get_json();
    if (event.get_type() == TYPE_EXTERNAL_CONTROL_TOUCH) {
      // Expected payload: {ID: "", ACTION: "update", DATA: {status: true, "name": "touch_0", ID: 0}}
      uint8_t id = stoi(default_value(data, ID, "0"));
      
      // reset timer update status  
      cnt_timer_ = 0;

      if (data.contains(ENABLED)) {
        auto enabledTouch = default_value(data, ENABLED, true);
        Publisher<ObjectEnableTouch>::publish(
          ObjectEnableTouch(
            id,
            enabledTouch
          )
        );
      }
      else {
        bool state = default_value(data, STATE, false);
        Publisher<DigitalOutputValue>::publish(
          DigitalOutputValue(
            id,
            state,
            false
          )
        );

        if (data.contains("dateTime")) {
          id_syc_ = default_value(data, "dateTime", "0");
        }
      }
      res2Server(data, action, event.get_id_package());
    }
    else if (event.get_type() ==  TYPE_EXTERNAL_SET_SCHEDULE) {
      int index = 0;
      uint8_t id = 0;
      bool state = false;
      bool repeat = false;
      bool notify = true;
      std::string cron = "";

      if (action == "insert") {
        if (data.contains(INDEX)) {
          index = default_value(data, INDEX, 0);
        }
        if (data.contains(ID)) {
          id = stoi(default_value(data, ID, "0"));
        }
        if (data.contains(STATE)) {
          state = default_value(data, STATE, false);
        }
        if (data.contains(REPEAT)) {
          repeat = default_value(data, REPEAT, false);
        }
        if (data.contains(NOTIFICATION)) {
          notify = default_value(data, NOTIFICATION, false);
        }
        if (data.contains(CRON)) {
          cron = default_value(data, CRON, "");
        }

        Publisher<ObjectSetTimer>::publish(
          ObjectSetTimer(
            index,
            id,
            state,
            repeat,
            notify,
            cron,
            action
          )
        );

        res2Server(data, action, event.get_id_package());
      }
      else if (action == "delete") {
        if (data.contains(INDEX)) {
          index = default_value(data, INDEX, 0);
        }
        if (data.contains(ID)) {
          id = stoi(default_value(data, ID, "0"));
        }
        
        Publisher<ObjectSetTimer>::publish(
          ObjectSetTimer(
            index,
            id,
            state,
            repeat,
            notify,
            cron,
            action
          )
        );

        res2Server(data, action, event.get_id_package());
      }
      else if (action == "get") {
        auto obj = nlohmann::json::object();
        auto schedules = storage::StorageSchedule::instance().get()[SCHEDULES];

        for (int i = 0; i < 4; i++) {
          auto num = std::to_string(i);
          if (!schedules[num][TIMER].empty()) {
            auto arr = nlohmann::json::array();
            for ( const auto &item :schedules[num][TIMER].items()) {
              arr.insert(arr.end(), item.value()[INDEX]);
            }
            obj[num] = arr;
          }
        }
      
        Publisher<ObjectDataDev2Ser>::publish(
          ObjectDataDev2Ser(
            TYPE_EXTERNAL_SET_SCHEDULE,
            "Update",
            obj
          )
        );
      }
    }
    else if (event.get_type() ==  TYPE_EXTERNAL_SET_COUNTDOWN) {
      uint8_t io = stoi(default_value(data, ID, "0"));
      bool state = default_value(data, STATE, false);
      std::string cron = default_value(data, CRON, "");

      Publisher<ObjectSetCountDown>::publish(ObjectSetCountDown(
        io,
        state,
        cron,
        action
      ));

      res2Server(data, action, event.get_id_package());
    }
    else if (event.get_type() == TYPE_EXTERNAL_PING_ICMP) {
      if (data.contains("ping")
          && data["ping"].contains("ip")) {
        auto ip = default_value(data["ping"], "ip", "google.com");
        int size_package = 5;
        if (data["ping"].contains("len")) {
          size_package = default_value(data["ping"], "len", 5);
        }

        Publisher<ObjectPingServer>::publish(
          ObjectPingServer(
            ip,
            size_package
          )
        );
      }

      res2Server(data, action, event.get_id_package());

    }
    else if (event.get_type() ==  TYPE_EXTERNAL_OTA) {
      std::string key = default_value(data, KEY, "");
      std::string version = default_value(data, VERSION, "");
      std::string environment = default_value(data, ENVIRONMENT, "");
      if (action == "update" 
          && key == DEFAULT_TYPE_KEY 
          && version != fireAlarm::DataCache::instance().get(VERSION)
          && environment == fireAlarm::DataCache::instance().get(ENVIRONMENT)) {
        std::string link;
        std::string::size_type pos = default_value(data, DATA, "").find("?key=");
        if (pos) {
          link = default_value(data, DATA, "").substr(0, pos);
        }
        else {
          link = default_value(data, DATA, "");
        }

        StorageNvsE::instance().write("link_ota", link);

        Publisher<ObjectDataSendToMetro>::publish(
          ObjectDataSendToMetro(
            TYPE_INTERNAL_RESET,
            "",
            {}
          )
        );
      }

      id_package = event.get_id_package();
    }
    else if (event.get_type() == TYPE_INTERNAL_SET_SCHEDULE) {
      int touch_id = default_value(data, "touch", 0);
      bool touch_state = default_value(data, "state", false);
      std::string touch_name = default_value(data, "name", "");

      Publisher<DigitalOutputValue>::publish(DigitalOutputValue(
        touch_id,
        touch_state
      ));
    }
    else if (event.get_type() == TYPE_EXTERNAL_SET_SLEEP_LED_RED) {
      if (data.contains("startTimer") && data.contains("endTimer")) {
        std::string startTimer = default_value(data, "startTimer", "");
        std::string endTimer = default_value(data, "endTimer", "");
        Publisher<led::EventSleepLedRed>::publish(
          led::EventSleepLedRed(startTimer, endTimer)
        );
      }
      res2Server(data, action, event.get_id_package());
    }
    else if (event.get_type() == TYPE_INTERNAL_RESET) {
      helper_.restartChip();
    }
    else if (event.get_type() == TYPE_INTERNALL_TOUCH_SETTING) {
      nlohmann::json v;
      v[VERSION] = fireAlarm::DataCache::instance().get(VERSION);
      v[MODEL] = fireAlarm::DataCache::instance().get(MODEL);
      v[ENVIRONMENT] = fireAlarm::DataCache::instance().get(ENVIRONMENT);
      v[PRODUCTION_DATE] = fireAlarm::DataCache::instance().get(PRODUCTION_DATE);
      v[CUSTOMER] = fireAlarm::DataCache::instance().get(CUSTOMER);
      v[MANUFACTURE] = fireAlarm::DataCache::instance().get(MANUFACTURE);
      v[FACTORY] = fireAlarm::DataCache::instance().get(FACTORY);
      v[IP] = fireAlarm::DataCache::instance().get("ip");

      Publisher<ObjectDataDev2Ser>::publish(
        ObjectDataDev2Ser(
          TYPE_EXTERNAL_SEND_TOUCH_SETTING,
          "Update",
          v
        )
      );
    }
    else if (event.get_type() == TYPE_EXTERNAL_STATUS_2_SERVER) {
      updateStatus(event.get_action());
    }
    
  } catch(...) {

  }
}

void MetroController::event(const DigitalInputValue& event)
{
  Log::info(METRO_TAG, "DigitalInputValue: id: {0}, value: {1}", event.get_input(), event.get_value());
  if (event.get_input() >= TOUCH_TYPE) {
    return;
  }

  cacheState[event.get_input()].id = event.get_input();
  cacheState[event.get_input()].value = event.get_value();
  cacheState[event.get_input()].is_push = true;
  cacheState[event.get_input()].timer = esp_timer_get_time();
  cacheState[event.get_input()].is_ack = event.get_is_ack();
  cnt_timer_ = 0;
}

void MetroController::event(const ObjectTypeButton& event)
{
  switch(event.get_type()) {
    case TypeButton::SMART:
    {
      if (fireAlarm::DataCache::instance().get(WIFI_MODE) 
            == std::to_string(WifiMode::smartconfig_mode) ||
          fireAlarm::DataCache::instance().get(WIFI_MODE) 
            == std::to_string(WifiMode::provisioning_mode)) {
        fireAlarm::DataCache::instance().set(WIFI_MODE, std::to_string(WifiMode::ap_mode));
        Publisher<ObjectModeWifi>::publish(
          ObjectModeWifi(WifiMode::ap_mode)
        );
      }
      else {
        fireAlarm::DataCache::instance().set(WIFI_MODE, std::to_string(WifiMode::smartconfig_mode));
        Publisher<ObjectModeWifi>::publish(
          ObjectModeWifi(WifiMode::smartconfig_mode)
        );
      }
    }
      break;
    case TypeButton::PROVISIONING:
    {
      if (fireAlarm::DataCache::instance().get(WIFI_MODE) 
            == std::to_string(WifiMode::smartconfig_mode) ||
          fireAlarm::DataCache::instance().get(WIFI_MODE) 
            == std::to_string(WifiMode::provisioning_mode)) {
        fireAlarm::DataCache::instance().set(WIFI_MODE, std::to_string(WifiMode::ap_mode));
        Publisher<ObjectModeWifi>::publish(
          ObjectModeWifi(WifiMode::ap_mode)
        );
      }
      else {
        fireAlarm::DataCache::instance().set(WIFI_MODE, std::to_string(WifiMode::provisioning_mode));
        Publisher<ObjectModeWifi>::publish(
          ObjectModeWifi(WifiMode::provisioning_mode)
        );
      }
    }
      break;
    case TypeButton::RESET_SYSTEM:
      helper_.restartChip();
      break;
    case TypeButton::REFACTORY_SYSTEM:
      Publisher<ObjectModeWifi>::publish(
        ObjectModeWifi(WifiMode::reset_mode)
      );
      break;
    case TypeButton::NONE:
      break;
    default:
      break;
  }

  cnt_reset_mode_timer_ = 0;
}

void MetroController::event(const ObjectMetroPingServer& event) 
{
  std::string type = TYPE_EXTERNAL_PING_ICMP;
  std::string action = "Update";
  nlohmann::json v;
  v["ping"] = event.get_data();
  Publisher<ObjectDataDev2Ser>::publish(
    ObjectDataDev2Ser(
      type,
      action,
      v
    )
  );
}

void MetroController::event(const smooth::core::timer::TimerExpiredEvent& event)
{
  if (fireAlarm::DataCache::instance().get(ACTIVATE) == "1") {
    cnt_timer_++;
    cnt_reset_mode_timer_++;
    if (cnt_timer_ >= MAX_TIMER_UPDATE_PING) {
      cnt_timer_ = 0;
      ping();
    }

    if (cnt_reset_mode_timer_ >= TIMER_15_MINUTES) {
      if (fireAlarm::DataCache::instance().get(WIFI_MODE) 
              == std::to_string(WifiMode::smartconfig_mode) ||
            fireAlarm::DataCache::instance().get(WIFI_MODE) 
              == std::to_string(WifiMode::provisioning_mode)) {

        Publisher<led::EventLed>::publish(
          led::EventLed("exit")
        );

        fireAlarm::DataCache::instance().set(WIFI_MODE, std::to_string(WifiMode::ap_mode));
        Publisher<ObjectModeWifi>::publish(
          ObjectModeWifi(WifiMode::ap_mode)
        );
      }
      cnt_reset_mode_timer_ = 0;

      if (is_to_long_ota_) {
        Publisher<led::EventLed>::publish(
          led::EventLed("exit_ota")
        );
        Publisher<DisableTouch>::publish(
          DisableTouch(false)
        );
        is_to_long_ota_ = false;
      }
    }

    // push state touch to server
    pushStateTouch();
  }
}

void MetroController::event(const EventPending& event)
{
  is_to_long_ota_ = event.get_state();
  nlohmann::json v;

  if (is_to_long_ota_) {
    v["status"] = 0;
    v["desc"] = "Started update firmware";
  }
  else {
    v["status"] = 2;
    std::string desc = "Update firmware failed: " + event.get_info();
    v["desc"] = desc;
  }

  Publisher<ObjectDataDev2Ser>::publish(
    ObjectDataDev2Ser(
      TYPE_EXTERNAL_OTA,
      "ack",
      v,
      id_package
    )
  );
}

void MetroController::updateStatus(const std::string ac)
{
  auto v = nlohmann::json::array();
  auto object = nlohmann::json::object();

  int32_t value_default = 0;
  for (size_t i = 0; i < TOUCH_TYPE; i++) {
    std::string key = "touch_" + std::to_string(i);
    if (StorageNvsE::instance().read(key.c_str(), &value_default)) {
      object[ID] = std::to_string(i);
      object[STATE] = value_default == 1 ? true : false;
      v.push_back(object);
    }
  }

  Publisher<ObjectDataDev2Ser>::publish(
    ObjectDataDev2Ser(
      TYPE_EXTERNAL_STATUS_2_SERVER,
      ac,
      v
    )
  );
}

void MetroController::ping()
{
  nlohmann::json v{};

  v["rssi"] = fireAlarm::DataCache::instance().get("rssi");
  v[ID] = StorageNvsE::instance().read("id_syc");

  int32_t cnt_value = 0;
#if TOUCH_TYPE  >= 1
  if (StorageNvsE::instance().read("cnt_touch_0", &cnt_value)) {
    v["numberTimesTouch_1"] = cnt_value;
  }
#if TOUCH_TYPE  >= 2
  if (StorageNvsE::instance().read("cnt_touch_1", &cnt_value)) {
    v["numberTimesTouch_2"] = cnt_value;
  }

#if TOUCH_TYPE  >= 3
  if (StorageNvsE::instance().read("cnt_touch_2", &cnt_value)) {
    v["numberTimesTouch_3"] = cnt_value;
  }
  
#if TOUCH_TYPE  >= 4
  if (StorageNvsE::instance().read("cnt_touch_3", &cnt_value)) {
    v["numberTimesTouch_4"] = cnt_value;
  }
#endif
#endif
#endif
#endif

  Publisher<ObjectDataDev2Ser>::publish(
    ObjectDataDev2Ser(
      TYPE_EXTERNAL_PING_2_SERVER,
      "Update",
      v
    )
  );
}

void MetroController::pushStateTouch()
{
  unsigned long now = esp_timer_get_time();

  for (int i = 0; i < TOUCH_TYPE; i++) {
    if (cacheState[i].is_push && (now - cacheState[i].timer >= DEBOUNCE_PUSH_STATE.count() * 1000)) {
      cacheState[i].is_push = false;
      nlohmann::json v;
      v[NAME] = "TOUCH_" + std::to_string(cacheState[i].id);
      v[ID] = std::to_string(cacheState[i].id);
      v[STATE] = cacheState[i].value;

      std::string type = cacheState[i].is_ack ? TYPE_EXTERNAL_ACK_2_SERVER : TYPE_EXTERNAL_CONTROL_TOUCH;
      
      if (type == TYPE_EXTERNAL_CONTROL_TOUCH) {
        auto time = duration_cast<seconds>(system_clock::now().time_since_epoch());
        id_syc_ = std::to_string(time.count());
      }

      Publisher<ObjectSetDigitalState2Storage>::publish(
        ObjectSetDigitalState2Storage(
          cacheState[i].id,
          cacheState[i].value,
          id_syc_
        )
      );
      
      Publisher<ObjectDataDev2Ser>::publish(
        ObjectDataDev2Ser(
          type,
          "Update",
          v
        )
      );
    }
  }
}

void MetroController::res2Server(const nlohmann::json& data, const std::string& action, const std::string& id)
{
  std::string type = TYPE_EXTERNAL_ACK_2_SERVER;
  Publisher<ObjectDataDev2Ser>::publish(
    ObjectDataDev2Ser(
      type,
      action,
      data,
      id
    )
  );
}

}