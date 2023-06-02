#include "DeviceId.h"
#include <random>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <smooth/core/json/JsonFile.h>
#include <smooth/core/logging/log.h>
#include <smooth/core/util/json_util.h>
#include <smooth/core/util/string_util.h>
#include <smooth/core/filesystem/SPIFlash.h>

using namespace smooth::core;
using namespace smooth::core::json;
using namespace smooth::core::json_util;
using namespace smooth::core::util;
using namespace smooth::core::logging;
using namespace smooth::core::filesystem;

namespace iotTouch
{
  static const auto id_file = FlashMount::instance().mount_point() / "dev_id.jsn";

  const std::string& DeviceId::get()
  {
    
    if(id.empty())
    {
      id = generate();
    }
    
    return id;
  }

  void DeviceId::write_default() const
  {    
    SPIFlash flash{ FlashMount::instance(), "app_storage", 5, true };
    if (flash.mount()) 
    {
      JsonFile f{id_file};

      if(!f.exists())
      {
        auto& v = f.value();

        const auto& generated = generate();
        v["device_id"] = generated;
        if(f.save())
        {
          // Log::info("Generated device ID", generated);
        }
        else
        {
          Log::error("Device ID", "Could not save device id.");
        }
      }
    }

    flash.unmount();
  }

  std::string DeviceId::generate() const
  {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0x0, 0xF);

    std::stringstream ss;
    for (int i = 0; i < 8; ++i)
    {
      ss << std::setfill('0') << std::setw(2) << std::hex << dis(gen);
    }

    return ss.str();
  }
}