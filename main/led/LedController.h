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
#include "common/EventLed.hpp"
#include <smooth/core/json/JsonFile.h>
#include <nlohmann/json.hpp>

namespace led
{
    class LedController 
        : public smooth::core::ipc::IEventListener<smooth::core::timer::TimerExpiredEvent>,
          public smooth::core::ipc::IEventListener<EventLed>
    {
        public:
            explicit LedController(smooth::core::Task&);

            void event(const smooth::core::timer::TimerExpiredEvent& ev) override;
            void event(const EventLed& ev) override;

        private:
            void on();
            void off();
            void play_song(const std::string& name);
            void start_song();
            void time_next_tone();
            void initialize();
            
            smooth::core::Task& task;

            using TimerExpiredQueue = smooth::core::ipc::TaskEventQueue<smooth::core::timer::TimerExpiredEvent>;
            using EventLedQueue = smooth::core::ipc::SubscribingTaskEventQueue<EventLed>;

            std::shared_ptr<TimerExpiredQueue> queue;
            std::shared_ptr<EventLedQueue> to_play;
            smooth::core::timer::TimerOwner timer;
            nlohmann::json note_book;
            nlohmann::json current_song{};
            std::list<std::chrono::milliseconds> song_timings{};            
            bool is_on{false};
    };
}