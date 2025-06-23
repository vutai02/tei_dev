#include "StorageInfoDevice.h"

#include <limits>
#include <smooth/core/logging/log.h>
#include <smooth/core/util/json_util.h>
#include "common/ConfigConstant.h"

using namespace fireAlarm::common;
using namespace smooth::core::json;
using namespace smooth::core::logging;
using namespace smooth::core::json_util;
using namespace smooth::core::filesystem;
namespace fireAlarm::storage
{

  StorageInfoDevice::StorageInfoDevice()
    :f_(FlashMount::instance().mount_point() / "_device.jsn")
  {
  }

  bool StorageInfoDevice::load()
  {
    return f_.load();
  }

  void StorageInfoDevice::save()
  {
    (void)f_.save();
  }

  smooth::core::filesystem::Path StorageInfoDevice::get_path()
  {
    return FlashMount::instance().mount_point() / "_device.jsn";
  }
  
  void StorageInfoDevice::write_default()
  {
    JsonFile jf{ FlashMount::instance().mount_point() / "_device.jsn" };

    auto& v = jf.value();
    auto& configuration = v[CONFIGURATION];
    
    configuration[ACTIVATE] = false;
    auto& device = configuration[DEVICE];
    device[ID]  = "";
    device[TOKEN] = "";

    auto& cloud = configuration[CLOUD_SKY_TECH];
    cloud[PORT] = DEFAULT_PORT_CLOUD;
    cloud[SERVER] = DEFAULT_ENDPOINT_CLOUD;
  
    auto& mqtt = configuration[MQTT];
    mqtt[ENABLE] = true;
    mqtt[MQTT_ENDPOINT] = DEFAULT_ENDPOINT_CLOUD;
    mqtt[MQTT_PORT] = DEFAULT_PORT_MQTT_CLOUD;
    mqtt[USER] = "";
    mqtt[PASS] = ""; 
    mqtt[SSL_KEY] = "";
    mqtt[SSL_CERT] = "";
    mqtt[CLIENT_ID] = "";
    mqtt[SSL_CA_CERT] = "";
    mqtt[KEEP_ALIVE] = 60;
    mqtt[CLEAN_SESSION] = true;
    mqtt[RECONNECT_TIMEOUT_MIN] = 2;
    mqtt[RECONNECT_TIMEOUT_MAX] = 60;
    mqtt[SCHEME] = "mqtt";

    auto& topic = configuration[TOPIC];
    for (int i = 0; i < 3; i++) {
      auto num = std::to_string(i);
      auto& tp = topic[num];
      tp[ID] = "";
      tp[NAME] = "";
    }
    
    if(!jf.save()) {
      Log::error("Config", "Could not save default config device.");
    }
    else {
      Log::info("Config", "save default config device successful.");
    }
  }

  bool StorageInfoDevice::isStorageInfoDeviceValid()
  {
    nlohmann::json device = f_.value();
    if (!device.contains(CONFIGURATION)) return false;
    
    if (!device[CONFIGURATION].contains(ACTIVATE)) return false;

    if (!device[CONFIGURATION].contains(DEVICE)) return false;
    if (!device[CONFIGURATION][DEVICE].contains(ID)) return false;
    if (!device[CONFIGURATION][DEVICE].contains(TOKEN)) return false;

    if (!device[CONFIGURATION].contains(CLOUD_SKY_TECH)) return false;
    if (!device[CONFIGURATION][CLOUD_SKY_TECH].contains(PORT)) return false;
    if (!device[CONFIGURATION][CLOUD_SKY_TECH].contains(SERVER)) return false;

    if (!device[CONFIGURATION].contains(MQTT)) return false;
    if (!device[CONFIGURATION][MQTT].contains(MQTT_PORT)) return false;
    if (!device[CONFIGURATION][MQTT].contains(MQTT_ENDPOINT)) return false;
    if (!device[CONFIGURATION][MQTT].contains(USER)) return false;
    if (!device[CONFIGURATION][MQTT].contains(PASS)) return false;
    if (!device[CONFIGURATION][MQTT].contains(SCHEME)) return false;

    return true;
  }

}
