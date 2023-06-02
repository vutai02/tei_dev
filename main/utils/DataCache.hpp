#pragma once

#include <string>
#include <map>
#include "common/ConfigConstant.h"
#include "storage/StorageInfoDevice.h"
#include "storage/nvs/StorageNvsE.h"
#include <smooth/core/json/JsonFile.h>
#include <smooth/core/util/json_util.h>

using namespace smooth::core::json;
using namespace smooth::core::util;
using namespace smooth::core::logging;
using namespace smooth::core::json_util;

using namespace iotTouch::storage;
using namespace iotTouch::common;

namespace iotTouch
{
  class DataCache
  {
    public:
      static DataCache& instance()
      {
        static DRAM_ATTR DataCache map_key{};
        return map_key;
      }

      DataCache& operator=(const DataCache&) = delete;
      DataCache& operator=(DataCache&&) = delete;
      DataCache(const DataCache&) = delete;
      DataCache(DataCache&&) = delete;

      void set(std::string key,  std::string value) {
        std::lock_guard<std::mutex> lock(guard);
        
        std::map<std::string, std::string>::iterator it;
        it = data.find(key);
        if (it != data.end()) {
          data[key] = value;
        }
        else {
          data[key] = value;
        }
      }

      std::string get(std::string key) {
        
        std::lock_guard<std::mutex> lock(guard);

        std::map<std::string, std::string>::iterator it;
        it = data.find(key);
        if (it != data.end()) {
          return it->second;
        }
        else {
          return {};
        }
      }

    private:
    DataCache() {
      initialize();
    }
    std::map<std::string, std::string> data;
    std::mutex guard;

    void initialize() {
      try {

        int32_t wifi_mode_;
        auto cache_ssid = StorageNvsE::instance().read(SSID);
        auto cache_pass = StorageNvsE::instance().read(KEY);
        StorageNvsE::instance().read(WIFI_MODE, &wifi_mode_);
        data[SSID] = cache_ssid;
        data[KEY] = cache_pass;
        data[WIFI_MODE] =  std::to_string(wifi_mode_);

        auto config = StorageInfoDevice::instance().get()[CONFIGURATION];
        
        data[CLOUD_PORT] = std::to_string(default_value(config[CLOUD_SKY_TECH], CLOUD_PORT, DEFAULT_PORT_CLOUD));
        data[CLOUD_ENDPOINT] = default_value(config[CLOUD_SKY_TECH], CLOUD_ENDPOINT, DEFAULT_ENDPOINT_CLOUD);
        
        data[EXTERNAL_ID] = default_value(config[DEVICE], ID, "");
        data[EXTERNAL_KEY] = default_value(config[DEVICE], TOKEN, "");

        data[MQTT_PORT] = std::to_string(default_value(config[MQTT], MQTT_PORT, DEFAULT_PORT_MQTT_CLOUD));
        data[MQTT_ENDPOINT] = default_value(config[MQTT], MQTT_ENDPOINT, DEFAULT_ENDPOINT_CLOUD);
        data[CLIENT_ID] = default_value(config[MQTT], CLIENT_ID, "");
        data[USER] = default_value(config[MQTT], USER, "");
        data[PASS] = default_value(config[MQTT], PASS, "");
        data[SCHEME] = default_value(config[MQTT], SCHEME, "mqtts");

        bool activate = default_value(StorageInfoDevice::instance().get()[CONFIGURATION], ACTIVATE, false);
        data[ACTIVATE] = activate == true ? "1": "0";

        for (int i = 0; i < config[TOPIC].size(); i++) {
          auto num = std::to_string(i);
          auto id = config[TOPIC][num][ID];
          auto name = config[TOPIC][num][NAME];
          data[name] = id;
        }
        data[VERSION] = ""; // version 
        data[MODEL] = ""; // 
        data[PRODUCTION_DATE] = "";
        data[MANUFACTURE] = "";
        data[FACTORY] = "";
        data["rssi"] = "0";
        data[IP] = "";
      } catch(...) {
      }
    }
  };
}