#pragma once
#include <string>

class EventPending
{
  public:
    EventPending() = default;
    EventPending(const bool& state, const std::string& info) : state_(state), info_(info) {}
    bool get_state() const { return state_; }
    std::string get_info() const { return info_; }
  private:
    bool state_{false};
    std::string info_{};
};