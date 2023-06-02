#pragma once

#include <string>

class ObjectSetDigitalState2Storage
{
  public:
    ObjectSetDigitalState2Storage() = default;

    ObjectSetDigitalState2Storage(uint8_t io, bool state, std::string id)
        : io(io),
          state(state),
          id(id)
    {
    }

    bool get_state() const
    {
      return state;
    }

    uint8_t get_io() const
    {
      return io;
    }

    std::string get_id() const
    {
      return id;
    }

  private:
    uint8_t io = 0;
    bool state = false;
    std::string id = {};
};
