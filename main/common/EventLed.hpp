#pragma once
#include <string>

namespace led
{
  class EventLed
  {
    public:
      EventLed() = default;
      EventLed(const std::string& name) : name(name) {}
      const std::string get_name() const { return name; }
    private:
      std::string name;
  };
}