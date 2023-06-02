#pragma once

#include <memory>
#include <chrono>
#include <smooth/core/sntp/Sntp.h>
#include <smooth/core/timer/Timer.h>
#include <smooth/core/Task.h>
#include <smooth/core/ipc/IEventListener.h>
#include <smooth/core/filesystem/SPIFlash.h>

namespace iotTouch
{
    namespace network
    {
      class Sntp : public smooth::core::ipc::IEventListener<smooth::core::timer::TimerExpiredEvent>
      {
      public:
          Sntp(smooth::core::Task &task);

          void start()
          {
            sntp_timer_->start();
          }

          void event(const smooth::core::timer::TimerExpiredEvent& ev) override;

      private:
          smooth::core::Task &task_;
          using SNTPQueue = smooth::core::ipc::TaskEventQueue<smooth::core::timer::TimerExpiredEvent>;
          std::shared_ptr<SNTPQueue> sntp_queue_;
          smooth::core::timer::TimerOwner sntp_timer_{};
          std::unique_ptr<smooth::core::sntp::Sntp> sntp_{};
      };
    }
}