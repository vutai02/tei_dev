#pragma once

#include <string>

class DigitalCountDown
{
  public:
    DigitalCountDown() = default;

    DigitalCountDown(uint8_t id, bool state, std::string cron = "", std::string action = "insert")
        : id(id),
          state(state),
          cron(cron),
          action(action)
    {
    }

    uint8_t get_id() const
    {
      return id;
    }

    bool get_state() const
    {
      return state;
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
    uint8_t id = 0;
    bool state = false;
    std::string cron{};
    std::string action{};
};
