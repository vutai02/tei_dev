#pragma once

#include <string>

// {"id": "", "action": "Insert", "data": {"index": 1, "id": 0, "cron": "*/15 * * * * *", "status": true, "repeat": false, "notify": true}}

class ObjectSetTimer
{
  public:
    ObjectSetTimer() = default;

    ObjectSetTimer(int index, uint8_t id, bool state, bool repeat, bool notify, std::string cron, std::string action)
        : index(index),
          id(id),
          state(state),
          repeat(repeat),
          notify(notify),
          cron(cron),
          action(action)
    {
    }

    int get_index() const
    {
      return index;
    }

    uint8_t get_id() const
    {
      return id;
    }
    bool get_state() const
    {
      return state;
    }

    bool get_repeat() const
    {
      return repeat;
    }

    bool get_notify() const
    {
      return notify;
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
    int index = 0;
    uint8_t id = 0;
    bool state = false;
    bool repeat = false;
    bool notify = false;
    std::string cron{};
    std::string action{};
};
