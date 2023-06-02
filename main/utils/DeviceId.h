#pragma once

#include <string>

namespace iotTouch
{
  class DeviceId
  {
    public:
      const std::string& get();
      void write_default() const;
    private:
      std::string generate() const;

      std::string id;
  };
}