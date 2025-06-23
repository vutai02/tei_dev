#include "StorageLevelInput.h"

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

    StorageLevelInput::StorageLevelInput()
        : f_(FlashMount::instance().mount_point() / "_vl.jsn")
    {
    }

    bool StorageLevelInput::load()
    {
        return f_.load();
    }

    void StorageLevelInput::save()
    {
        (void)f_.save();
    }

    void StorageLevelInput::write_default()
    {
        JsonFile jf{FlashMount::instance().mount_point() / "_vl.jsn"};

        auto &v = jf.value();
        // Config the default input for level
        auto &levelinput = v[LEVEL_INPUT];
        for (int i = 0; i < 15; i++)
        {
            auto num = std::to_string(i);
            {
                levelinput["input" + num] = false;
            }
        }

        if (!jf.save())
        {
            Log::error("Config", "Could not save default config level input.");
        }
        else
        {
            Log::info("Config", "save default config level input successful.");
        }
    }

    smooth::core::filesystem::Path StorageLevelInput::get_path()
    {
        return FlashMount::instance().mount_point() / "_vl.jsn";
    }

    bool StorageLevelInput::isStorageLevelInputValid()
    {
        nlohmann::json sch = f_.value();
        if (!sch.contains(LEVEL_INPUT))
            return false;
        return true;
    }
}