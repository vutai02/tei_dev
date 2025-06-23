#include "rs485/core_master/Core.h"
using namespace std::chrono;
namespace RS485
{
    static constexpr const char *CORE_TAG = "core_rs485";
    mb_param_request_t request;
    Core_RS485::Core_RS485()
    {
        Log::info(CORE_TAG, "constructor this object");
    }

    static esp_err_t master_init(void)
    {
        mb_communication_info_t comm = {
            .mode = MB_MODE_RTU,
            .slave_addr = 0x01,
            .port = MB_PORT_NUM,
            .baudrate = MB_DEV_SPEED,
            .parity = MB_PARITY_NONE};
        void *master_handler = NULL;
        esp_err_t err = mbc_master_init(MB_PORT_SERIAL_MASTER, &master_handler);

        err |= mbc_master_setup((void *)&comm);
        // Set UART pin numbers
        err |= uart_set_pin(MB_PORT_NUM, CONFIG_MB_UART_TXD, CONFIG_MB_UART_RXD,
                            UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

        err |= mbc_master_start();

        err |= uart_set_mode(MB_PORT_NUM, UART_MODE_RS485_APP_CTRL);

        return err;
    }
    void Core_RS485::config_UART_RS485()
    {
        esp_err_t res;
        int count = 0;
        do
        {
            res = master_init();
            count++;
            if (count == 5)
            {
                Log::info(CORE_TAG, "fail init master Rs");
                //----> send to server about sattus of pheriral
            }
        } while (res != ESP_OK);
    }


    bool Core_RS485::read_modbus(uint16_t reg_start, int32_t &raw_data, uint16_t size_res, uint32_t time_delay)
    {
        vTaskDelay(pdMS_TO_TICKS(time_delay));
        esp_err_t res;
        // uint32_t result = 0;
        int count = 0;
        request = {
            .slave_addr = SLAVE,
            .command = COMMAND_3,
            .reg_start = reg_start,
            .reg_size = size_res};
    retry:
        res = (mbc_master_send_request(&request, &raw_data));
        if (res == ESP_OK)
        {
            // Log::info(TAG, "RS485 successfully");
            return true;
        }
        else if (res != ESP_OK)
        {
            // retry 5times
            count++;
            vTaskDelay(pdMS_TO_TICKS(time_delay * 5));
            if (count > 5)
            {
                // raw_data.push_back(-1); // reading error
                raw_data = 0;
                return false;
            }
            goto retry;
        }
        return true;
    }
    bool Core_RS485::write_modbus(uint16_t res_start, uint16_t size_res, uint16_t *data)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
        esp_err_t res;
        request = {
            .slave_addr = SLAVE,
            .command = COMMAND_16,
            .reg_start = res_start,
            .reg_size = size_res};
        res = (mbc_master_send_request(&request, data));
        return true;
    }

    bool Core_RS485::read_modbus_32bit(uint16_t reg_start, uint16_t *raw_data, uint16_t size_res, uint32_t time_delay)
    {
        vTaskDelay(pdMS_TO_TICKS(time_delay));
        esp_err_t res;
        // uint32_t result = 0;
        int count = 0;
        request = {
            .slave_addr = SLAVE,
            .command = COMMAND_3,
            .reg_start = reg_start,
            .reg_size = size_res};
    retry:
        res = (mbc_master_send_request(&request, raw_data));
        if (res == ESP_OK)
        {
            return true;
        }
        else if (res != ESP_OK)
        {
            // retry 5times
            count++;
            vTaskDelay(pdMS_TO_TICKS(time_delay * 5));
            if (count > 5)
            {
                // raw_data.push_back(-1); // reading error
                raw_data = 0;
                return false;
            }
            goto retry;
        }
        return true;
    }

}