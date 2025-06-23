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
#include "rs485/core_master/Core.h"
// #include "rs485/core_slave/slave.h"
#include "define_res.h"
#include "common/ConstantType.h"
#include "freertos/semphr.h"

using namespace std;
using namespace std::chrono;
using namespace fireAlarm::common;
using namespace smooth::core::util;
using namespace smooth::core::ipc;
using namespace smooth::core::logging;
using namespace smooth::core::json_util;

typedef struct {
    uint8_t magicNumber;   // Magic number to identify the packet
    uint8_t packetLength;  // Length of the packet (including this byte)
    uint8_t req;           // Request or command byte
    uint8_t* payload;      // Pointer to the payload data
    uint16_t crc16;        // CRC16 checksum for the packet
    uint8_t data;           // Device ID or Identifier (optional)
} Packet; 

namespace fireAlarm
{
    class MBTask : public smooth::core::Task
    {
    public:
        MBTask();
        void tick() override;
        void init() override;
        void read_io(struct RS485::parameter_ATS::io_parameter *data);
        void write_io(struct RS485::parameter_ATS::io_parameter *data);
    private:
        RS485::Core_RS485 Core_RS485_{};
        RS485::parameter_ATS parameter_ATS_;
        int32_t type_mode = 0;
        int32_t pre_mode = 0;
    };
}