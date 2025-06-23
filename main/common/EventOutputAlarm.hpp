#pragma once
#include <string>
namespace outputAlarm
{
    class EventOutputAlarm
    {
    public:
        EventOutputAlarm() = default;
        EventOutputAlarm(const std::string &name) : name(name) {}
        const std::string get_name() const { return name; }

    private:
        std::string name;
    };
}