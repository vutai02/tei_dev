#include "utils/CommandDispatcher.h"

namespace iotTouch
{
  void CommandDispatcher::process(const std::string& command, const std::string& data, const std::string& id_package)
  {
    auto it = execs.find(command);
    if( it != execs.end())
    {
      it->second(command, data, id_package);
    }
  }

  void CommandDispatcher::add_command(const std::string& command_topic, Executor executor)
  {
    execs.emplace(command_topic, executor);
  }
}