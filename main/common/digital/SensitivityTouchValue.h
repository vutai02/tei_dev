#pragma once

#include <string>

class SensitivityTouchValue
{
  public:
    SensitivityTouchValue() = default;

    SensitivityTouchValue(uint8_t id, float value = 0)
        : id(id), value(value)
    {
    }

    uint8_t get_id() const
    {
      return id;
    }

    float get_value() const
    {
      return value;
    }

  private:
    uint8_t id;
    float value;
};