#pragma once

#include <string>

// {"id": "", "action": "Update", "data": {"id": 0, "cron": "*/15 * * * * *", "status": true}}

class ObjectSetCountDown
{
  public:
    ObjectSetCountDown() = default;

    ObjectSetCountDown(uint8_t id, bool state, std::string cron = "", std::string action = "")
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
    bool repeat = false;
    std::string cron{};
    std::string action{};
};
