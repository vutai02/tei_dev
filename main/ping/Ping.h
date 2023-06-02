#pragma once

#include <chrono>
#include <memory>
#include <list>
#include <smooth/core/Task.h>
#include <smooth/core/timer/Timer.h>
#include <smooth/core/timer/TimerExpiredEvent.h>
#include <smooth/core/ipc/TaskEventQueue.h>
#include <smooth/core/ipc/SubscribingTaskEventQueue.h>
#include <smooth/core/filesystem/SPIFlash.h>
#include "common/ObjectPingServer.h"
#include <smooth/core/json/JsonFile.h>
#include <nlohmann/json.hpp>

namespace ping
{
  class Ping 
    : public smooth::core::ipc::IEventListener<ObjectPingServer>
  {
    public:
      explicit Ping(smooth::core::Task&);

      void event(const ObjectPingServer& ev) override;

    private:
      void stop();
      void start(const std::string& add, int len);
      smooth::core::Task& task;

      using ObjectPingServerQueue = smooth::core::ipc::SubscribingTaskEventQueue<ObjectPingServer>;

      std::shared_ptr<ObjectPingServerQueue> ping;
  };
}