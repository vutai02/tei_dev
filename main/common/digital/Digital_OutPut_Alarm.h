#pragma once

#include <string>

class Digital_OutPut_Alarm
{
public:
    Digital_OutPut_Alarm() = default;

    Digital_OutPut_Alarm(uint8_t io, bool status, std::string action = "")
        : io(io),
          status(status),
          action(action)
    {
    }

    uint8_t get_io() const
    {
        return io;
    }

    bool get_status() const
    {
        return status;
    }
    const std::string &get_action() const
    {
        return action;
    }

private:
    uint8_t io = 0;
    bool status = false;
    std::string action{};
};