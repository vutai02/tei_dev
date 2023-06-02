#pragma once
#include <string>

namespace led
{
  class EventSleepLedRed
  {
    public:
      EventSleepLedRed() = default;
      EventSleepLedRed(const std::string& start, const std::string& end) : startTimer(start), endTimer(end) {}
      const std::string get_start() const { return startTimer; }
      const std::string get_end() const { return endTimer; }
    private:
      std::string startTimer;
      std::string endTimer;
  };
}