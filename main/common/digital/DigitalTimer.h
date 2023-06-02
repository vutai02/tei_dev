#pragma once

#include <string>

class DigitalTimer
{
  public:
    DigitalTimer() = default;

    DigitalTimer(uint8_t input, bool value, bool repeat, int index, std::string cron = "", std::string action = "insert")
        : input(input),
          value(value),
          repeat(repeat),
          index(index),
          cron(cron),
          action(action)
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

    bool get_repeat() const
    {
      return repeat;
    }
    
    int get_index() const
    {
      return index;
    }

    const std::string& get_cron() const
    {
      return cron;
    }

    const std::string& get_action() const
    {
      return action;
    }

  private:
    uint8_t input = 0;
    bool value = false;
    bool repeat = false;
    int index = 0;
    std::string cron{};
    std::string action{};
};
