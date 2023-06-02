#pragma once

#include <string>
#include <unordered_map>
#include <functional>

namespace iotTouch
{
  typedef std::function<void(const std::string& command, const std::string& data, const std::string& id_package)> Executor;

  class CommandDispatcher
  {
    public:
      void add_command(const std::string& command_topic, Executor executor);
      void process(const std::string& command, const std::string& data, const std::string& id_package = "");
    private:
      std::unordered_map<std::string, Executor> execs{};
  };
}