#include "StorageWifi.h"

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

  StorageWifi::StorageWifi()
    :f_(FlashMount::instance().mount_point() / "_wifi.jsn")
  {
  }

  bool StorageWifi::load()
  {
    return f_.load();
  }

  void StorageWifi::save()
  {
    (void)f_.save();
  }

  smooth::core::filesystem::Path StorageWifi::get_path()
  {
    return FlashMount::instance().mount_point() / "_wifi.jsn";
  }

  void StorageWifi::write_default()
  {
    JsonFile jf{ FlashMount::instance().mount_point() / "_wifi.jsn" };
    
    auto& v = jf.value();
    v[WIFI_MODE] = WifiMode::ap_mode;

    auto& sta = v[STA];
    sta[ENABLE] = true;
    sta[SSID] = "";
    sta[KEY] = "";
    sta[MAC_ADDR] = "";

    if(!jf.save()) {
      Log::error("Config", "Could not save default config wifi.");
    }
    else {
      Log::info("Config", "save default config wifi successful.");
    }
  }

  bool StorageWifi::isStorageWifiValid()
  {
    nlohmann::json wifi = f_.value();
    if (!wifi.contains(WIFI_MODE)) return false;
    if (!wifi.contains(STA)) return false;
    if (!wifi[STA].contains(ENABLE)) return false;
    if (!wifi[STA].contains(SSID)) return false;
    if (!wifi[STA].contains(KEY)) return false;
    if (!wifi[STA].contains(MAC_ADDR)) return false;
    return true;
  }
}
