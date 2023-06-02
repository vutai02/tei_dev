#pragma once

#include <string>

class ObjectStatus
{
  public:
    ObjectStatus() = default;

    ObjectStatus(std::string objJson)
        : objJson(objJson)
    {
    }

    std::string get_objJson() const
    {
      return objJson;
    }

  private:
    std::string objJson{};
};
