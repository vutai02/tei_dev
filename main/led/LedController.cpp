#include "LedController.h"

#include <string>
#include <smooth/core/ipc/Publisher.h>
#include <smooth/core/util/json_util.h>
#include "utils/timer_id.h"
#include <smooth/core/filesystem/MountPoint.h>
#include "common/digital/DigitalIoLed.h"
#include "common/ConstantType.h"

using namespace smooth::core::ipc;
using namespace smooth::core::timer;
using namespace smooth::core::filesystem;
using namespace smooth::core::json_util;
using namespace smooth::core::json;
using namespace smooth::core::logging;
using namespace iotTouch::common;

namespace led
{

  LedController::LedController(smooth::core::Task& task)
    : task(task),
    queue(TimerExpiredQueue::create(3, task, *this)),
    to_play(EventLedQueue::create(3, task, *this)),
    timer(STROBE_LIGHT_TICK, queue, false, std::chrono::milliseconds{0})
  {
    initialize();
  }

  void LedController::initialize()
  {
    note_book = R"(
      {
        "smart": {
        "signal": [1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000],
        "repeat": true
        },
        "provisioning": {
        "signal": [500, 500, 500, 500, 500, 500, 500, 500],
        "repeat": true
        },
        "refactor": {
        "signal": [100, 100, 100, 100, 100, 100],
        "repeat": true
        },
        "receive_data": {
        "signal": [400, 50, 400, 50, 400, 50],
        "repeat": true
        },
        "activated": {
        "signal": [1000],
        "repeat": false
        },
        "error": {
        "signal": [250, 250, 250, 250, 250, 250, 250, 250],
        "repeat": false
        },
        "entry_ota": {
        "signal": [200, 200, 200, 200, 200, 200],
        "repeat": true
        },
        "exit_ota": {
        "signal": [2500, 2500, 2500, 2500, 2500],
        "repeat": false
        },
        "exit": {
        "signal": [100],
        "repeat": false
        }
      }
    )"_json;
  }

  void LedController::on()
  {
    is_on = true;
    // publisher to io
    Publisher<DigitalIoLed>::publish(
      DigitalIoLed(
        true
      )
    );
  }

  void LedController::off()
  {
    is_on = false;
    // publisher to io
    Publisher<DigitalIoLed>::publish(
      DigitalIoLed(
        false
      )
    );
  }

  void LedController::event(const smooth::core::timer::TimerExpiredEvent& ev)
  {
    if (!song_timings.empty())
    {
      // Alternate between on/off each time the timer expires.
      if (is_on)  {
        off();
      }
      else {
        on();
      }

      time_next_tone();
    }
    else {
      off();

      if(default_value(current_song, "repeat", false)) {
        // Log::info("LedController", "Repeating io led");
        start_song();
      }         
    }        
  }

  void LedController::event(const EventLed& ev)
  {
    auto song = note_book.find(ev.get_name());

    if(song != note_book.end()) {
      current_song = *song;
      play_song(ev.get_name());
    }
    else  {
      // Log::info("LedController","'{}' not found", ev.get_name());
      off();
      song_timings.clear();
      current_song.clear();
    }  
  }

  void LedController::play_song(const std::string& name)
  {
    Log::info("LedController", "start: '{}'", name);
    start_song();
  }

  void LedController::start_song()
  {
    off();

    song_timings.clear();

    auto signal = current_song["signal"];
    for(auto t : signal)  {
      song_timings.emplace_back(t);
    }

    if(!song_timings.empty()) {
      on();
      time_next_tone();
    }
  }

  void LedController::time_next_tone()
  {
    timer->start(song_timings.front());
    song_timings.pop_front();
  }
}