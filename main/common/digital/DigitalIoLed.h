#pragma once


class DigitalIoLed
{
  public:
    DigitalIoLed() = default;

    DigitalIoLed(bool state)
        : state(state)
    {
    }

    bool get_state() const
    {
      return state;
    }
  private:
    bool state {false};
};
