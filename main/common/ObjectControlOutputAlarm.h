#pragma once

#include <string>
#include <nlohmann/json.hpp>

class ObjectControlOutputAlarm
{
public:
    ObjectControlOutputAlarm() = default;

    ObjectControlOutputAlarm(const std::string &type, const std::string &action, nlohmann::json json)
        : type(type),
          action(action),
          v(json)
    {
    }

    const std::string &get_type() const
    {
        return type;
    }

    const std::string &get_action() const
    {
        return action;
    }

    const nlohmann::json &get_json() const
    {
        return v;
    }

private:
    std::string type{};
    std::string action{};
    nlohmann::json v{};
};