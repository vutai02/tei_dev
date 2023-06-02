#pragma once

#include <string>

class SyncTimerSer
{
  public:
    SyncTimerSer() = default;

    SyncTimerSer(bool state):state(state)
    {
    }

    uint8_t get_state() const
    {
      return state;
    }

  private:
    bool state = false;
};
