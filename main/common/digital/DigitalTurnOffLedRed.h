#pragma once

class DigitalTurnOffLedRed
{
  public:
    DigitalTurnOffLedRed() = default;

    DigitalTurnOffLedRed(bool state)
        : state(state)
    {
    }

    bool get_state_led() const
    {
      return state;
    }

  private:
    bool state {false};
};
