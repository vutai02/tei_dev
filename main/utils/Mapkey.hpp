#pragma once

#include <string>
#include <map>
#include "common/ConfigConstant.h"

namespace fireAlarm
{
  class MapKey
  {
    public:
      static MapKey& instance()
      {
        static MapKey map_key{};
        return map_key;
      }

      MapKey& operator=(const MapKey&) = delete;
      MapKey& operator=(MapKey&&) = delete;
      MapKey(const MapKey&) = delete;
      MapKey(MapKey&&) = delete;

    uint8_t convert(std::string key) {
      for (auto it = data.begin(); it != data.end(); ++it) {
        if (it->second == key)
            return it->first;
      }
      return 0;
    }

    std::string convert(uint8_t value) {
      std::map<uint8_t, std::string>::iterator it;
      it = data.find(value);
      if (it != data.end()) {
        return it->second;
      } else return {};
    }

    private:
    MapKey() {
      initialize();
    }
    std::map<uint8_t, std::string> data;

    void initialize() {
      data[0] = "unknown";
    }
  };
}