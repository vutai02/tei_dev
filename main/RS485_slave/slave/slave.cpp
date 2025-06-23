#include "RS485_slave/RS485.h"

using namespace std::chrono;
using namespace RS485;
using namespace fireAlarm::common;

namespace RS485 {
    static constexpr const char *CORE_SLAVE = "slave_rs485";

    bool Slave_RS485::initialized = false;
    uint16_t Slave_RS485::holding_regs[HOLDING_REG_SIZE] = {0};
    
    Slave_RS485::Slave_RS485() {
        for (int i = 0; i < HOLDING_REG_SIZE; i++) {
            holding_regs[i] = 0;
        }
    }
        
    Slave_RS485::~Slave_RS485() {
         deinit_slave();
    }
    
    void task_usb_uart(void* arg) {
        uint8_t data_buffer[256]; 
        int len = 0;
        while (true) {
            // Đọc dữ liệu từ UART RS485
            len = uart_read_bytes(MB_PORT_NUM, data_buffer, sizeof(data_buffer), pdMS_TO_TICKS(1000));  // Đọc tối đa 256 byte trong vòng 1 giây
            if (len > 0) {
                ESP_LOGI(CORE_SLAVE, "Received %d bytes", len);
                for (int i = 0; i < len; i++) {
                    ESP_LOGI(CORE_SLAVE, "Data[%d]: 0x%02x", i, data_buffer[i]);
                }
    
                if (data_buffer[0] == RS485::parameter_ATS::COMMAND_OUT1) {
                    uint16_t io_status = data_buffer[1];  
                    gpio_set_level(IO_OUTPUT_ALARM_2, io_status);
                    Log::info(CORE_SLAVE, "Set GPIO_OUTPUT_ALARM_2 to %d", io_status);
                }
                else if (data_buffer[0] == RS485::parameter_ATS::COMMAND_OUT2) {
                    uint16_t io_status = data_buffer[1]; 
                    gpio_set_level(IO_OUTPUT_ALARM_3, io_status);
                    Log::info(CORE_SLAVE, "Set GPIO_OUTPUT_ALARM_3 to %d", io_status);
                }
            }
            const char* msg = "Hello from RS485 Slave!";
            uart_write_bytes(MB_PORT_NUM, msg, strlen(msg));
            Log::info(CORE_SLAVE, "Sent data: %s", msg);
            vTaskDelay(pdMS_TO_TICKS(100));
            
        }
    }

    bool Slave_RS485::init_slave() {
        if (initialized) return true;
    
        mb_communication_info_t comm = {
            .mode = MB_MODE_RTU,
            .slave_addr = 0x01,
            .port = MB_PORT_NUM,
            .baudrate = MB_DEV_SPEED,    
            .parity = MB_PARITY_NONE
        };

        esp_err_t err = mbc_slave_init(MB_PORT_SERIAL_SLAVE, &slave_handler);
        if (err != ESP_OK) {
            Log::info(CORE_SLAVE, "Slave init failed: %s", esp_err_to_name(err));
            return false;
        }

        err = mbc_slave_setup((void*)&comm);
        if (err != ESP_OK) {
            Log::info(CORE_SLAVE, "Slave setup failed: %s", esp_err_to_name(err));
            return false;
        }

        mb_register_area_descriptor_t reg_area = {
            .start_offset = 0,
            .type = MB_PARAM_HOLDING,
            .address = (void*)holding_regs,
            .size = sizeof(holding_regs)
        };
        mbc_slave_set_descriptor(reg_area);

        err = mbc_slave_start();
        if (err != ESP_OK) {
            Log::info(CORE_SLAVE, "Slave start failed: %s", esp_err_to_name(err));
            return false;
        }

        err = uart_set_mode(MB_PORT_NUM, UART_MODE_RS485_HALF_DUPLEX);
        if (err != ESP_OK) {
            Log::info(CORE_SLAVE, "UART mode set failed: %s", esp_err_to_name(err));
            return false;
        }
    
        initialized = true;
        Log::info(CORE_SLAVE, "Slave initialized (Address: %d, Baud: %d)", 0X01, MB_DEV_SPEED);
        
        xTaskCreate(task_usb_uart, "task_usb_uart", 1024 * 5, NULL, 2, NULL);


        return true;
    }
    
    void Slave_RS485::deinit_slave() {
        if (initialized) {
            mbc_slave_destroy();
            initialized = false;
            Log::info(CORE_SLAVE, "Slave deinitialized");
        }
    }
    
    esp_err_t Slave_RS485::read_holding_registers_cb(uint8_t addr, uint16_t start, uint16_t* regs) {
        if (addr != 0X01 || start >= HOLDING_REG_SIZE) {
            return ESP_ERR_INVALID_ARG;
        }
        
        for (int i = 0; i < HOLDING_REG_SIZE - start; i++) {
            regs[i] = holding_regs[start + i];
        }
        
        Log::info(CORE_SLAVE, "Read Holding Registers [%d-%d]", start, start + (HOLDING_REG_SIZE - start - 1));
        return ESP_OK;
    }

    esp_err_t Slave_RS485::write_multiple_holding_registers_cb(uint8_t addr, uint16_t start, uint16_t n_regs, uint16_t* data) {
        if (addr != 0X01 || start + n_regs > HOLDING_REG_SIZE) {
            return ESP_ERR_INVALID_ARG;
        }

        for (int i = 0; i < n_regs; i++) {
            uint16_t reg_addr = start + i;
            holding_regs[reg_addr] = data[i];

            switch (reg_addr) {
                case RS485::parameter_ATS::COMMAND_OUT1:
                    gpio_set_level(IO_OUTPUT_ALARM_2, data[i]);
                    Log::info(CORE_SLAVE, "Đặt GPIO_OUTPUT_ALARM_2 thành %d", data[i]);
                    break;
                case RS485::parameter_ATS::COMMAND_OUT2:
                    gpio_set_level(IO_OUTPUT_ALARM_3, data[i]);
                    Log::info(CORE_SLAVE, "Đặt GPIO_OUTPUT_ALARM_3 thành %d", data[i]);
                    break;
                default:
                    break;
            }
        }

        Log::info(CORE_SLAVE, "Written %d Holding Registers from addr %d", n_regs, start);
        return ESP_OK;
    }
}