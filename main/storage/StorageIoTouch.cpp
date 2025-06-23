#include "StorageIoTouch.h"

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
  std::string path_io_touch = static_cast<std::string>(DEFAULT_TYPE_KEY) + "_io_touch.jsn";
  static const auto storage_io_touch_file =
    FlashMount::instance().mount_point() / path_io_touch;

  StorageIoTouch::StorageIoTouch()
    :f_(storage_io_touch_file)
  {
  }

  void StorageIoTouch::init()
  {
    write_default();
    if (load()) {
      if (f_.value().dump() == "null") {
        write_default(true);
      }

      if (!isStorageIoTouchValid()) {
        write_default(true);
      }
    }
    else {
      write_default(true);
    }
  }

  bool StorageIoTouch::load()
  {
    return f_.load();
  }

  void StorageIoTouch::save()
  {
    (void)f_.save();
  }

  const char* StorageIoTouch::get_path()
  {
    return storage_io_touch_file;
  }


  void StorageIoTouch::write_default(bool isWrite)
  {
    JsonFile jf{ storage_io_touch_file };
    if (!jf.exists() || isWrite) {
      auto& v = jf.value();
      // Config sensor touch
      auto& sensor =  v[SENSORS];
      for (int i = 0; i < 4; i++) {
        auto num = std::to_string(i);
        {
          auto& o = sensor[num];
          o[NAME] = "TOUCH_" + num;
          o[ID] = num;
          o[ENABLED] = true;
          o[STATE] = false;
        }
      }
      v[ID] = "";

      if(!jf.save()) {
        Log::error("Config", "Could not save default config io touch.");
      }
      else {
        Log::info("Config", "save default config io touch successful.");
      }
    }
  }

  bool StorageIoTouch::isStorageIoTouchValid()
  {
    nlohmann::json io = f_.value();
    if (!io.contains(SENSORS)) return false;
    for (int i = 0; i < 4; i++) {
      auto num = std::to_string(i);
      {
        if (!io[SENSORS].contains(num)) return false;
        if (!io[SENSORS][num].contains(NAME)) return false;
        if (!io[SENSORS][num].contains(ID)) return false;
        if (!io[SENSORS][num].contains(ENABLED)) return false;
        if (!io[SENSORS][num].contains(STATE)) return false;
      }
    }
    return true;
  }
}
