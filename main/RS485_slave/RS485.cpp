#include <memory>
#include <smooth/core/util/ByteSet.h>
#include <smooth/core/ipc/Publisher.h>
#include <smooth/core/util/json_util.h>
#include <smooth/core/task_priorities.h>
#include "utils/timer_id.h"
#include <driver/gpio.h>
#include "freertos/FreeRTOS.h"
#include <nlohmann/json.hpp>
#include "freertos/portmacro.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "smooth/core/SystemStatistics.h"
#include "RS485.h"

using namespace smooth::core;
using namespace std::chrono;
using namespace smooth::core::timer;
using namespace smooth::core::util;
using namespace smooth::core::ipc;
using namespace smooth::core::logging;
using namespace smooth::core::json_util;
using RS485::Slave_RS485;


namespace fireAlarm
{
    static constexpr const char *SLAVE_TAG = "slave_rs485";
    MBTask::MBTask()
        : Task(SLAVE_TAG, 4 * 1024, 5, milliseconds{3000}),
        slave(std::make_shared<Slave_RS485>())
    {
    }

    void MBTask::tick()
    {
        Log::info(SLAVE_TAG, "tick"); 
    }

    void MBTask::init()
    {
        Log::info(SLAVE_TAG, "init");

        if (!is_initialized)
        {
            Log::info(SLAVE_TAG, "Initializing slave RS485...");
            if (slave->init_slave())
            {
                is_initialized = true;
                Log::info(SLAVE_TAG, "RS485 Slave initialized successfully.");
            }
            else
            {
                Log::error(SLAVE_TAG, "RS485 Slave init failed.");
            }
        }

        
    }
}