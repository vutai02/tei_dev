#pragma once

#include <string>


class ObjectPingServer
{
  public:
    ObjectPingServer() = default;

    ObjectPingServer(const std::string& add, int len = 5)
        : add_(add), len_(len)
    {
    }

    const std::string get_add() const
    {
      return add_;
    }

    int get_len() const
    {
      return len_;
    }

  private:
    std::string add_ = {};
    int len_;
};
