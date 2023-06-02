#pragma once

#include <string>
#include <nlohmann/json.hpp>

class ObjectDataSendToMetro
{
public:
  ObjectDataSendToMetro() = default;

  ObjectDataSendToMetro(const std::string& type, const std::string& action, nlohmann::json json, const std::string& id = "")
      : type(type),
        action(action),
        v(json),
        id_package(id)
  {
  }

  const std::string& get_type() const
  {
    return type;
  }

  const std::string& get_action() const
  {
    return action;
  }

  const nlohmann::json& get_json() const
  {
    return v;
  }

  const std::string& get_id_package() const
  {
    return id_package;
  }

private:
  std::string type{};
  std::string action{};
  nlohmann::json v{};
  std::string id_package{};
};
