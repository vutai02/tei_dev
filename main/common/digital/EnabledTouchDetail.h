#pragma once

#include <string>

class DisableTouch
{
  public:
    DisableTouch() = default;

    DisableTouch(bool state)
        : state(state)
    {
    }

    bool get_state() const
    {
      return state;
    }

  private:
    bool state;
};

class EnabledTouchDetail
{
  public:
    EnabledTouchDetail() = default;

    EnabledTouchDetail(uint8_t id, bool state = false)
        : id(id), state(state)
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

  private:
    uint8_t id;
    bool state;
};