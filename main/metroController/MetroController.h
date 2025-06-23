#pragma once 

#include <memory>
#include <tuple>
#include <smooth/core/timer/Timer.h>
#include <smooth/core/Application.h>
#include <smooth/core/ipc/Publisher.h>
#include <smooth/core/io/InterruptInput.h>
#include <smooth/core/ipc/ISRTaskEventQueue.h>
#include <smooth/core/timer/TimerExpiredEvent.h>
#include <smooth/core/ipc/SubscribingTaskEventQueue.h>
#include "common/digital/DigitalInputValue.h"
#include "common/ObjectDataSendToMetro.h"
#include "common/ObjectTypeButton.h"
#include "common/SystemHelper.h"
#include "common/IoInterface.h"
#include "common/ConfigConstant.h"
#include "common/ObjectMetroPingServer.h"
#include "common/EventPending.hpp"
#include "utils/Utils.h"

namespace fireAlarm
{
  using DatatoMetroQueue = smooth::core::ipc::SubscribingTaskEventQueue<ObjectDataSendToMetro>;
  using DigitalInputValueQueue = smooth::core::ipc::SubscribingTaskEventQueue<DigitalInputValue>;
  using ObjectTypeButtonQueue = smooth::core::ipc::SubscribingTaskEventQueue<ObjectTypeButton>;
  using ObjectMetroPingServerQueue = smooth::core::ipc::SubscribingTaskEventQueue<ObjectMetroPingServer>;
  using EventPendingQueue = smooth::core::ipc::SubscribingTaskEventQueue<EventPending>;
  using ExpiredQueue = smooth::core::ipc::TaskEventQueue<smooth::core::timer::TimerExpiredEvent>;

  class MetroController
      : public smooth::core::Task,
        public smooth::core::ipc::IEventListener<ObjectDataSendToMetro>,
        public smooth::core::ipc::IEventListener<DigitalInputValue>,
        public smooth::core::ipc::IEventListener<ObjectTypeButton>,
        public smooth::core::ipc::IEventListener<ObjectMetroPingServer>,
        public smooth::core::ipc::IEventListener<EventPending>,
        public smooth::core::ipc::IEventListener<smooth::core::timer::TimerExpiredEvent>
      
  {
    public:
      MetroController();

      void tick() override;
      void init() override;

      void event(const ObjectDataSendToMetro& event) override;
      void event(const DigitalInputValue& event) override;
      void event(const ObjectTypeButton& event) override;
      void event(const ObjectMetroPingServer& event) override;
      void event(const EventPending& event) override;
      void event(const smooth::core::timer::TimerExpiredEvent& event) override;
      void updateStatus(const std::string ac = "Update");
      void ping();
    private:

      std::shared_ptr<DatatoMetroQueue> data_to_metro_;
      std::shared_ptr<DigitalInputValueQueue> digital_input_;
      std::shared_ptr<ObjectTypeButtonQueue> type_button_;
      std::shared_ptr<ObjectMetroPingServerQueue> ping_ser_;
      std::shared_ptr<ExpiredQueue> timer_queue_;
      smooth::core::timer::TimerOwner timer_;
      int cnt_timer_ = 0;
      int cnt_reset_mode_timer_ = 0;
      fireAlarm::Utils utils_;
      fireAlarm::system::SystemHelper helper_;
      std::string id_package {};
      std::string id_syc_ {};
      void res2Server(const nlohmann::json& v, const std::string& action = "Update", const std::string& id = "");
      void pushStateTouch();
      iot_tp_dev_t cacheState[TOUCH_TYPE];
      bool is_to_long_ota_{false};
  };
}