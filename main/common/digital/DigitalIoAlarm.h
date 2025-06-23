#pragma once

class DigitalIoAlarm
{
public:
    DigitalIoAlarm() = default;

    DigitalIoAlarm(bool state, bool disabled = false)
        : state(state), disabled(disabled)
    {
    }

    bool get_state() const
    {
        return state;
    }

    bool get_disabled() const
    {
        return disabled;
    }

private:
    bool state{false};
    bool disabled{false};
};