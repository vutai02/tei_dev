
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
#include "rs485.h"

using namespace smooth::core;
using namespace std::chrono;
using namespace smooth::core::timer;
using namespace smooth::core::util;
using namespace smooth::core::ipc;
using namespace smooth::core::logging;
using namespace smooth::core::json_util;

namespace fireAlarm
{
    // RS485 use modbus type RTU
    static constexpr const char *MB_TAG = "modbus";
    static int32_t checking_range_data(int64_t min, int64_t max, int32_t ptr);
    SemaphoreHandle_t myMutex = NULL;
    MBTask::MBTask()
        : Task(MB_TAG, 4 * 1024, 5, milliseconds{3000})
    {
    }
    void MBTask::tick()
    {
        Log::info(MB_TAG, "tick");
        read_io(&parameter_ATS_.io_Vals); 
        Log::info(MB_TAG, "gpio_in1: %d", parameter_ATS_.io_Vals.gpio_out1);
        write_io(&parameter_ATS_.io_Vals);
        Log::info(MB_TAG, "gpio_out2: %d", parameter_ATS_.io_Vals.gpio_out2);
      
    }

    void MBTask::init()
    {
        Log::info(MB_TAG, "init");
        Core_RS485_.config_UART_RS485();
    }

    static int32_t checking_range_data(int64_t min, int64_t max, int32_t ptr)
    {
        if (ptr > max || ptr < min)
        {

            Log::info(MB_TAG, "Data out of range: %d (Expected: %lld <= %d <= %lld)", ptr, min, ptr, max);
            
            ptr = 0;
        }
        return ptr;
    }

    void MBTask::read_io(struct RS485::parameter_ATS::io_parameter *data)
    {
        int32_t temp;
        Core_RS485_.read_modbus(parameter_ATS_.COMMAND_OUT1, temp, 1, 25);
        data->gpio_out1 = static_cast<uint16_t>(checking_range_data(0, 1, temp));
        Core_RS485_.read_modbus(parameter_ATS_.COMMAND_OUT2, temp, 1, 25);
        data->gpio_out2 = static_cast<uint16_t>(checking_range_data(0, 1, temp));
    }

    void MBTask::write_io(struct RS485::parameter_ATS::io_parameter *data)
    {
        uint16_t io1_status = static_cast<uint16_t>(data->io1_state);
        uint16_t io2_status = static_cast<uint16_t>(data->io2_state);
  
        bool result1 = Core_RS485_.write_modbus(parameter_ATS_.COMMAND_OUT1, 1, &io1_status);  
        if (result1)
        {
            Log::info(MB_TAG, "IO1 updated status: %d", io1_status);
        }
        else
        {
            Log::error(MB_TAG, "Failed to update IO1 status: %d", io1_status);
        }
    
        bool result2 = Core_RS485_.write_modbus(parameter_ATS_.COMMAND_OUT2, 1, &io2_status);  
        if (result2)
        {
            Log::info(MB_TAG, "IO2 updated status: %d", io2_status);
        }
        else
        {
            Log::error(MB_TAG, "Failed to update IO2 status: %d", io2_status);
        }
    }

}
