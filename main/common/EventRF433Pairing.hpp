#pragma once
#include <string>

namespace event
{
  class EventRF433Pairing
  {
    public:
      EventRF433Pairing() = default;
      EventRF433Pairing(const std::string &type_code, const std::string &device_id) : type_code_(type_code), device_id_(device_id) {}
      std::string device_id_ser() const { return device_id_; }
      std::string device_type() const { return type_code_; }
    private:
      std::string type_code_ = "";
      std::string device_id_ = "";
  };
}