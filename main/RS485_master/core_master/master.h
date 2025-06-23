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
#define CONFIG_MB_UART_RXD 16
#define CONFIG_MB_UART_TXD 15
#define MB_PORT_NUM (1)
#define MB_DEV_SPEED 19200

using namespace smooth::core::logging;
namespace Master_rs485
{
    typedef struct {
    uint8_t slave_addr;
    uint8_t command;
    uint16_t reg_start;
    uint16_t reg_size;
} mb_param_request_t;

    class Master_RS485
    {
    public:
        Master_RS485();
        void config_UART_RS485();
        bool read_modbus(uint16_t reg_start, int32_t &raw_data, uint16_t size_res, uint32_t time_delay);
        bool write_modbus(uint16_t res_start, uint16_t size_res, uint16_t *data);
        bool read_modbus_32bit(uint16_t reg_start, uint16_t *raw_data, uint16_t size_res, uint32_t time_delay);
    };

    esp_err_t mbc_master_init(mb_port_type_t port_type, void** handler);
    esp_err_t mbc_master_start(void);
    esp_err_t mbc_master_destroy(void);
    esp_err_t mbc_master_set_descriptor(const mb_parameter_descriptor_t *descriptor, const uint16_t num_elements);
    esp_err_t mbc_serial_master_send_request(mb_param_request_t* request, void* data_ptr);

}  
