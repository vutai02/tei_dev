#pragma once

#include <string>


class ObjectMetroPingServer
{
  public:
    ObjectMetroPingServer() = default;

    ObjectMetroPingServer(const std::string& data)
        : data(data)
    {
    }

    const std::string get_data() const
    {
      return data;
    }

  private:
    std::string data = {};
};
