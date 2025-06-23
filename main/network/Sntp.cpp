#include "Sntp.h"
#include <chrono>
#include <smooth/core/ipc/Publisher.h>
#include <smooth/core/json/JsonFile.h>
#include <smooth/core/logging/log.h>
#include <smooth/core/util/string_util.h>
#include "common/ConfigConstant.h"
#include "common/SyncTimerSer.h"
#include <smooth/core/util/json_util.h>

using namespace fireAlarm::common;
using namespace std::chrono;
using namespace smooth::core::ipc;
using namespace smooth::core::json;
using namespace smooth::core::logging;
using namespace smooth::core::json_util;
using namespace smooth::core::filesystem;

namespace fireAlarm
{
  namespace network
  {
    static const char* name = "SNTP";

    Sntp::Sntp(smooth::core::Task &task) :
      task_(task),
      sntp_queue_(SNTPQueue::create( 1, task, *this)),
      sntp_timer_(1, sntp_queue_, true, seconds{10})
    {
    }

    void Sntp::event(const smooth::core::timer::TimerExpiredEvent& ev)
    {
      if (!sntp_) {
        Log::info(name, "Starting SNTP");
        std::vector <std::string> servers{};
        servers.push_back("0.se.pool.ntp.org");
        servers.push_back("1.se.pool.ntp.org");
        servers.push_back("2.se.pool.ntp.org");

        if(!servers.empty()) {
          sntp_ = std::make_unique<smooth::core::sntp::Sntp>(servers);
          sntp_->start();
        } else {
          Log::info(name, "No SNTP servers configured");                    
        }
        
      } else if (sntp_->is_time_set()) {
        auto t = system_clock::to_time_t(system_clock::now());
        setenv("TZ", "WIB-7", 1);
        tzset();

        tm time{};
        localtime_r(&t, &time);
        Log::info(name, "Time set: {}", asctime(&time));

        Publisher<SyncTimerSer>::publish(SyncTimerSer(true));

        sntp_timer_->stop();
      } else {
        Log::info(name, "Waiting for time to be set");
      }
    }
  }
}