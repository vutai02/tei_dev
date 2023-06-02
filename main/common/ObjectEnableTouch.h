#pragma once

#include <string>

class ObjectEnableTouch
{
  public:
    ObjectEnableTouch() = default;

    ObjectEnableTouch(uint8_t io, bool state)
        : io(io),
          state(state)
    {
    }

    bool get_enabled_touch() const
    {
      return state;
    }

    uint8_t get_io() const
    {
      return io;
    }

  private:
    uint8_t io = 0;
    bool state = false;
};
