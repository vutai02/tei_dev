#pragma once
#include <string>

namespace event
{
  class EventPairing
  {
    public:
      EventPairing() = default;
      EventPairing(const bool &is_execute) :is_execute_(is_execute) {}
      bool is_execute() const { return is_execute_; }
    private:
      bool is_execute_ = false;
  };
}