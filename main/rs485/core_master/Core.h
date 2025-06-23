
#include <smooth/core/logging/log.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "driver/uart.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "soc/uart_struct.h"
#include "driver/gpio.h"
#include "mbcontroller.h"
#include "sdkconfig.h"
#include <smooth/core/util/ByteSet.h>
#include <smooth/core/ipc/Publisher.h>
#include <smooth/core/util/json_util.h>
#include <smooth/core/task_priorities.h>

#define SLAVE 0x01
#define CONFIG_MB_UART_RXD 16 //18
#define CONFIG_MB_UART_TXD 15 //17
#define MB_PORT_NUM (1)
#define MB_DEV_SPEED 19200

using namespace smooth::core::logging;
namespace RS485
{
    class Core_RS485
    {
    private:
        static constexpr uint8_t COMMAND_3 = 0X03;
        static constexpr uint8_t COMMAND_16 = 0X10;
        bool status = false;
    public:
        Core_RS485();
        void config_UART_RS485();
        bool write_modbus(uint16_t res_start, uint16_t size_res, uint16_t *data);
        bool read_modbus(uint16_t reg_start, int32_t &raw_data, uint16_t size_res, uint32_t time_delay);
        bool read_modbus_32bit(uint16_t reg_start, uint16_t *raw_data, uint16_t size_res, uint32_t time_delay);
    };
}