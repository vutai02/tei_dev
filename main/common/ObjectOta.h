#pragma once

#include <string>

class ObjectOta
{
  public:
    ObjectOta() = default;

    ObjectOta(bool mode, std::string urlOta = "")
        : mode(mode), urlOta(urlOta)
    {
    }

    bool get_mode() const
    {
      return mode;
    }

    const std::string& get_url() const
    {
      return urlOta;
    }
  
  private:
    bool mode = false;
    std::string urlOta{};
};
