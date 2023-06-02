#pragma once

#include <string>

class ObjectUpdateStatus
{
  public:
    ObjectUpdateStatus() = default;

    ObjectUpdateStatus(bool state):state(state)
    {
    }

    uint8_t get_state() const
    {
      return state;
    }

  private:
    bool state = false;
};
