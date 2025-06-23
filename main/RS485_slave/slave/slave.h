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
#include "RS485_slave/define.h"
#include "common/ConfigConstant.h"
#include "esp_modbus_slave.h"


#define SLAVE 0x01
#define CONFIG_MB_UART_RXD 18
#define CONFIG_MB_UART_TXD 17
#define MB_PORT_NUM (1)
#define MB_DEV_SPEED 19200
#define HOLDING_REG_SIZE  64 

namespace RS485 {

    class Slave_RS485 {
    public:
        Slave_RS485();
        ~Slave_RS485();

        bool init_slave();   
        void deinit_slave();
        static esp_err_t read_holding_registers_cb(uint8_t addr, uint16_t start, uint16_t* regs);
        static esp_err_t write_multiple_holding_registers_cb(uint8_t addr, uint16_t start, uint16_t n_regs, uint16_t* data);
        static bool initialized; 
        static uint16_t holding_regs[HOLDING_REG_SIZE];
    private:
        void* slave_handler = nullptr;
        // static bool initialized;
    };
} 

