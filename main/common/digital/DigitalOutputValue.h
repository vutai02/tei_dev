#pragma once

#include <string>

class DigitalOutputValue
{
  public:
    DigitalOutputValue() = default;

    DigitalOutputValue(uint8_t output, bool value, bool is_local = true)
        : output(output),
          value(value),
          local(is_local)
    {
    }

    uint8_t get_output() const
    {
      return output;
    }

    bool get_value() const
    {
      return value;
    }

    bool get_local() const
    {
      return local;
    }

  private:
    uint8_t output = 0;
    bool value = false;
    bool local = true;
};
