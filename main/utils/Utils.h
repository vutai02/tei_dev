#pragma once

#include <vector>
#include <string>

namespace fireAlarm
{
  class Utils
  {
    public:
      void toupper(std::string& data);
      void tolower(std::string& data);
      std::string convertHextoString(std::vector<uint8_t> data);
      std::vector<uint8_t> convertHexStrToBytes(std::string hex);
      int getVersion(const std::string& data);
    private:
  };
}