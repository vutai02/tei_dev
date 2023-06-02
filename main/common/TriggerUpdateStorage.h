#pragma once

class TriggerUpdateStorage
{
  public:
    TriggerUpdateStorage() = default;

    TriggerUpdateStorage(bool trigger)
        : trigger(trigger)
    {
    }

    bool get_trigger() const
    {
      return trigger;
    }

  private:
    bool trigger = {true};
};
