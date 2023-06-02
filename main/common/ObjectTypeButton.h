#pragma once

#include <string>

enum class TypeButton
{
  SMART             = 0,
  PROVISIONING      = 1,
  RESET_SYSTEM      = 2,
  REFACTORY_SYSTEM  = 3,
  NONE
};

class ObjectTypeButton
{
  public:
    ObjectTypeButton() = default;

    ObjectTypeButton(TypeButton type)
        : type(type)
    {
    }

    TypeButton get_type() const
    {
      return type;
    }
  
  private:
    TypeButton type{TypeButton::NONE};
};