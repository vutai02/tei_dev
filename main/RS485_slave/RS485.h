#pragma once

#include <memory>
#include <tuple>
#include <smooth/core/Application.h>
#include <smooth/core/ipc/ISRTaskEventQueue.h>
#include <smooth/core/timer/Timer.h>
#include <smooth/core/timer/TimerExpiredEvent.h>
#include <smooth/core/ipc/SubscribingTaskEventQueue.h>
#include <smooth/core/ipc/Publisher.h>
#include "smooth/core/Application.h"
#include "smooth/core/task_priorities.h"
#include <nlohmann/json.hpp>
#include "common/ConfigConstant.h"
// #include "RS485_slave/define.h"
#include "common/ConstantType.h"
#include "freertos/semphr.h"
#include "RS485_slave/slave/slave.h"

using namespace std;
using namespace std::chrono;
using namespace fireAlarm::common;
using namespace smooth::core::util;
using namespace smooth::core::ipc;
using namespace smooth::core::logging;
using namespace smooth::core::json_util;

namespace fireAlarm
{
class MBTask : public smooth::core::Task
{
public: 
    MBTask();
    void tick() override;
    void init() override;
private:
    bool is_initialized = false;
    std::shared_ptr<RS485::Slave_RS485> slave;
};
}
