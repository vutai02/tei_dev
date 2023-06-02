#pragma once

#include <string>

class DigitalInputValue
{
  public:
    DigitalInputValue() = default;

    DigitalInputValue(uint8_t input, bool value, bool is_ack = false, std::string char_name = "TOUCH_")
        : input(input),
          value(value),
          is_ack(is_ack),
          name(char_name + std::to_string(input))
    {
    }

    uint8_t get_input() const
    {
      return input;
    }

    bool get_value() const
    {
      return value;
    }

    bool get_is_ack() const
    {
      return is_ack;
    }

    const std::string& get_name() const
    {
      return name;
    }

  private:
    uint8_t input = 0;
    bool value = false;
    bool is_ack = false;
    std::string name{};
};
