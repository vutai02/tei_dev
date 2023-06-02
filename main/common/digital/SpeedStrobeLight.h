#pragma once

#include <string>

class SpeedStrobeLight
{
  public:
    SpeedStrobeLight() = default;

    SpeedStrobeLight(int speed)
        : speed(speed)
    {
    }

    int get_speed() const
    {
      return speed;
    }

  private:
    int speed = 0;
};
