#pragma once

#include <string>
#include <nlohmann/json.hpp>

class ObjectDataDev2Ser
{
public:
  ObjectDataDev2Ser() = default;

  ObjectDataDev2Ser(const std::string& type, const std::string& action, nlohmann::json json, const std::string& id = "", const std::string& method = "")
      : type(type),
        action(action),
        v(json),
        id(id),
        method(method)
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

  const std::string& get_id_package() const
  {
    return id;
  }

  const std::string& get_method() const
  {
    return method;
  }

  const nlohmann::json& get_json() const
  {
    return v;
  }

private:
  std::string type{};
  std::string action{};
  nlohmann::json v{};
  std::string id{};
  std::string method{};
};
