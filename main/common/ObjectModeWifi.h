#pragma once

#include <string>

class ObjectModeWifi
{
  public:
    ObjectModeWifi(int state = 0)
        : state(state)
    {
    }

    int get_mode() const
    {
      return state;
    }

  private:
    int state = 0;
};
