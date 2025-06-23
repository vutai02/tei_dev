#include "StorageSchedule.h"

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
  StorageSchedule::StorageSchedule()
    :f_(FlashMount::instance().mount_point() / "_sch.jsn")
  {
  }

  bool StorageSchedule::load()
  {
    return f_.load();
  }

  void StorageSchedule::save()
  {
    (void)f_.save();
  }

  void StorageSchedule::write_default()
  {
    JsonFile jf{ FlashMount::instance().mount_point() / "_sch.jsn" };

    auto& v = jf.value();
    // Config schedules
    for (int i = 0; i < TOUCH_TYPE; i++) {
      auto num = std::to_string(i);          
      {
        v[SCHEDULES][num][COUNTDOWN][ENABLE] = true;
        v[SCHEDULES][num][COUNTDOWN][STATE] = false;
        v[SCHEDULES][num][COUNTDOWN][TIMER] = "";
        v[SCHEDULES][num][COUNTDOWN][REPEAT] = false;

        v[SCHEDULES][num][TIMER] = nlohmann::json::array();
      }
    }
    v["sleepRed"]["start"] = "";
    v["sleepRed"]["end"] = "";

    if(!jf.save()) {
      Log::error("Config", "Could not save default config schedule.");
    }
    else {
      Log::info("Config", "save default config schedule successful.");
    }
  }
  
  smooth::core::filesystem::Path StorageSchedule::get_path()
  {
    return FlashMount::instance().mount_point() / "_sch.jsn";
  }

  bool StorageSchedule::isStorageScheduleValid()
  {
    nlohmann::json sch = f_.value();
    
    if (!sch.contains(SCHEDULES)) return false;
    for (int i = 0; i < TOUCH_TYPE; i++) {
      auto num = std::to_string(i);
      {
        if (!sch[SCHEDULES].contains(num)) return false;
        if (!sch[SCHEDULES][num].contains(TIMER)) return false;
        if (!sch[SCHEDULES][num].contains(COUNTDOWN)) return false;
        if (!sch[SCHEDULES][num][COUNTDOWN].contains(ENABLE)) return false;
        if (!sch[SCHEDULES][num][COUNTDOWN].contains(STATE)) return false;
        if (!sch[SCHEDULES][num][COUNTDOWN].contains(TIMER)) return false;
        if (!sch[SCHEDULES][num][COUNTDOWN].contains(REPEAT)) return false;
      }
    }
    if (!sch.contains("sleepRed")) return false;
    if (!sch["sleepRed"].contains("start")) return false;
    if (!sch["sleepRed"].contains("end")) return false;
    return true;
  }
}
