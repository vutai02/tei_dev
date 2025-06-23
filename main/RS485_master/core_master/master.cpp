#include "RS485_master/core_master/master.h"

using namespace std::chrono;
static mb_master_interface_t* mbm_interface_ptr = NULL;
namespace Master_rs485
{
    static constexpr const char *MASTER_TAG = "core_rs485";
    mb_param_request_t request;
    Master_RS485::Master_RS485()
    {
        Log::info(MASTER_TAG, "constructor this object");
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
        do {
            res = master_init();
            count++;
            if (count == 5) {
                Log::info(MASTER_TAG, "Failed to init master RF");
                // Send to server about status of peripheral
                break;
            }
        } while (res != ESP_OK);
    }
    
    static esp_err_t mbc_master_start(void)
    {
        if (mbm_interface_ptr == NULL) {
            ESP_LOGI(MASTER_TAG, "mbc_master_start: mbm_interface_ptr is NULL");
            return ESP_ERR_INVALID_STATE;
        }

        esp_err_t err = ESP_OK;
        eMBErrorCode mb_status = eMBMasterSerialInit((eMBMode)mbm_interface_ptr->opts.mbm_comm.mode,
                                                      (UCHAR)mbm_interface_ptr->opts.mbm_comm.port,
                                                      (ULONG)mbm_interface_ptr->opts.mbm_comm.baudrate,
                                                      MB_PORT_PARITY_GET(mbm_interface_ptr->opts.mbm_comm.parity));

        if (mb_status != MB_ENOERR) {
            ESP_LOGE(MASTER_TAG, "mbc_master_start: eMBMasterSerialInit failed");
            return ESP_ERR_INVALID_STATE;
        }

        mb_status = eMBMasterEnable();
        if (mb_status != MB_ENOERR) {
            ESP_LOGE(MASTER_TAG, "mbc_master_start: eMBMasterEnable failed");
            return ESP_ERR_INVALID_STATE;
        }

        ESP_LOGI(MASTER_TAG, "mbc_master_start: eMBMasterEnable success");
        return ESP_OK;
    }

    static esp_err_t mbc_master_destroy(void)
    {
        MB_MASTER_CHECK((mbm_interface_ptr != NULL),
                        ESP_ERR_INVALID_STATE,
                        "Master interface uninitialized.");
        mb_master_options_t* mbm_opts = &mbm_interface_ptr->opts;
        
        EventBits_t flags = xEventgroupClearBits(mbm_opts->mbm_event_group, MB_EVENT_STACK_STARTED);
        MB_MASTER_CHECK((flags & MB_EVENT_STACK_STARTED),
                        ESP_ERR_INVALID_STATE, "mb stack stop event failure.");

        eMBErrorCode mb_error = eMBMasterDisable();
        MB_MASTER_CHECK((mb_error == MB_ENOERR), 
                        ESP_ERR_INVALID_STATE, 
                        "mb stack disable failure.");
        
        if ( mbm_opts->mbm_task_handle)
        {
            vTaskDelete(mbm_opts->mbm_task_handle);
            mbm_opts->mbm_task_handle = NULL;
        }

        if(mbm_opts->mbm_event_group)
        {
            vEventGroupDelete(mbm_opts->mbm_event_group);
            mbm_opts->mbm_event_group = NULL;            
        }
    
        if(mbm_opts->mbm_sema)
        {
            vSemaphoreDelete(mbm_opts->mbm_sema);
            mbm_opts->mbm_sema = NULL;
        }

        mb_error = eMBMASTERClose();
        MB_MASTER_CHECK((mb_error == MB_ENOERR), 
                        ESP_ERR_INVALID_STATE,
                        "mb stack close failure returned (0x%x).", (int)mb_error);
        free(mbm_interface_ptr); // free the memory allocated for options
        vMBPortSetMode((UCHAR)MB_PORT_INACTIVE);
        mbm_interface_ptr = NULL;
        Log::info(MASTER_TAG, "mbc_master_destroy: mbm_interface_ptr is NULL");
        return ESP_OK;
    }

    static esp_err_t mbc_master_set_descriptor(const mb_parameter_descriptor_t *descriptor, const uint16_t num_elements)
        {
            if (descriptor == NULL || num_elements == 0 || mbm_interface_ptr == NULL) {
                return ESP_ERR_INVALID_ARG;
            }

            mb_master_options_t* mbm_opts = &mbm_interface_ptr->opts;

            for (uint16_t i = 0; i < num_elements; i++) {
                const mb_parameter_descriptor_t* reg_ptr = &descriptor[i];

                if (reg_ptr->cid != i || reg_ptr->param_key == NULL || reg_ptr->mb_size == 0) {
                    return ESP_ERR_INVALID_ARG;
                }
            }

            mbm_opts->mbm_param_descriptor_table = descriptor;
            mbm_opts->mbm_param_descriptor_size = num_elements;

            return ESP_OK;
        }


            
    static esp_err_t mbc_serial_master_set_request(char* name, mb_param_mode_t mode,
                                                   mb_param_request_t* request,
                                                   mb_parameter_descriptor_t* reg_data)
    {
        if (!mbm_interface_ptr || !name || !request || mode > MB_PARAM_WRITE) {
            return ESP_ERR_INVALID_ARG;
        }

        mb_master_options_t* mbm_opts = &mbm_interface_ptr->opts;
        if (mbm_opts->mbm_param_descriptor_table == NULL) {
            return ESP_ERR_INVALID_STATE;
        }

        const mb_parameter_descriptor_t* table = mbm_opts->mbm_param_descriptor_table;
        for (uint16_t i = 0; i < mbm_opts->mbm_param_descriptor_size; ++i) {
            const mb_parameter_descriptor_t* reg_ptr = &table[i];

            if (strcmp(name, reg_ptr->param_key) == 0) {
                if (reg_ptr->mb_slave_addr == 0 && mode == MB_PARAM_READ) {
                    return ESP_ERR_INVALID_ARG;
                }

                request->slave_addr = reg_ptr->mb_slave_addr;
                request->reg_start  = reg_ptr->mb_reg_start;
                request->reg_size   = reg_ptr->mb_size;
                request->command    = mbc_serial_master_get_command(reg_ptr->mb_param_type, mode);

                if (request->command == 0) {
                    return ESP_ERR_INVALID_ARG;
                }

                if (reg_data != NULL) {
                    *reg_data = *reg_ptr;
                }

                return ESP_OK;
            }
        }

        return ESP_ERR_NOT_FOUND;
    }
 

    static esp_err_t mbc_serial_master_send_request(mb_param_request_t* request, void* data_ptr)
    {
        MB_MASTER_CHECK((mbm_interface_ptr != nullptr),
                        ESP_ERR_INVALID_STATE,
                        "Master interface uninitialized.");
        mb_master_options_t* mbm_opts = &mbm_interface_ptr->opts;

        MB_MASTER_CHECK((request != nullptr),
                        ESP_ERR_INVALID_ARG, "mb request structure.");
        MB_MASTER_CHECK((data_ptr != nullptr),
                        ESP_ERR_INVALID_ARG, "mb incorrect data pointer.");

        eMBMasterReqErrCode mb_error = MB_MRE_MASTER_BUSY;
        esp_err_t error = ESP_FAIL;

        if (xSemaphoreTake(mbm_opts->mbm_sema, MB_SERIAL_API_RESP_TICS) == pdTRUE) {

            uint8_t mb_slave_addr = request->slave_addr;
            uint8_t mb_command = request->command;
            uint16_t mb_offset = request->reg_start;
            uint16_t mb_size = request->reg_size;

            mbm_opts->mbm_reg_buffer_ptr = static_cast<uint8_t*>(data_ptr);
            mbm_opts->mbm_reg_buffer_size = mb_size;

            switch (mb_command) {
                case MB_FUNC_OTHER_REPORT_SLAVEID:
                    mb_error = eMBMasterReqReportSlaveID(mb_slave_addr, static_cast<LONG>(MB_SERIAL_API_RESP_TICS));
                    break;

                case MB_FUNC_READ_COILS:
                    mb_error = eMBMasterReqReadCoils(mb_slave_addr, mb_offset, mb_size, static_cast<LONG>(MB_SERIAL_API_RESP_TICS));
                    break;

                case MB_FUNC_WRITE_SINGLE_COIL:
                    mb_error = eMBMasterReqWriteCoil(mb_slave_addr, mb_offset,
                                                    *static_cast<uint16_t*>(data_ptr),
                                                    static_cast<LONG>(MB_SERIAL_API_RESP_TICS));
                    break;

                case MB_FUNC_WRITE_MULTIPLE_COILS:
                    mb_error = eMBMasterReqWriteMultipleCoils(mb_slave_addr, mb_offset, mb_size,
                                                            static_cast<uint8_t*>(data_ptr),
                                                            static_cast<LONG>(MB_SERIAL_API_RESP_TICS));
                    break;

                case MB_FUNC_READ_DISCRETE_INPUTS:
                    mb_error = eMBMasterReqReadDiscreteInputs(mb_slave_addr, mb_offset, mb_size,
                                                            static_cast<LONG>(MB_SERIAL_API_RESP_TICS));
                    break;

                case MB_FUNC_READ_HOLDING_REGISTER:
                    mb_error = eMBMasterReqReadHoldingRegister(mb_slave_addr, mb_offset, mb_size,
                                                            static_cast<LONG>(MB_SERIAL_API_RESP_TICS));
                    break;

                case MB_FUNC_WRITE_REGISTER:
                    mb_error = eMBMasterReqWriteHoldingRegister(mb_slave_addr, mb_offset,
                                                                *static_cast<uint16_t*>(data_ptr),
                                                                static_cast<LONG>(MB_SERIAL_API_RESP_TICS));
                    break;

                case MB_FUNC_WRITE_MULTIPLE_REGISTERS:
                    mb_error = eMBMasterReqWriteMultipleHoldingRegister(mb_slave_addr, mb_offset, mb_size,
                                                                        static_cast<uint16_t*>(data_ptr),
                                                                        static_cast<LONG>(MB_SERIAL_API_RESP_TICS));
                    break;

                case MB_FUNC_READWRITE_MULTIPLE_REGISTERS:
                    mb_error = eMBMasterReqReadWriteMultipleHoldingRegister(mb_slave_addr, mb_offset, mb_size,
                                                                            static_cast<uint16_t*>(data_ptr),
                                                                            mb_offset, mb_size,
                                                                            static_cast<LONG>(MB_SERIAL_API_RESP_TICS));
                    break;

                case MB_FUNC_READ_INPUT_REGISTER:
                    mb_error = eMBMasterReqReadInputRegister(mb_slave_addr, mb_offset, mb_size,
                                                            static_cast<LONG>(MB_SERIAL_API_RESP_TICS));
                    break;

                default:
                    ESP_LOGI(TAG, "%s: Incorrect function in request (%u)", __FUNCTION__, static_cast<unsigned>(mb_command));
                    mb_error = MB_MRE_NO_REG;
                    break;
            }
        } else {
            ESP_LOGI(TAG, "%s: MBC semaphore take fail.", __FUNCTION__);
        }

        // Chuyển lỗi sang esp_err_t
        switch (mb_error) {
            case MB_MRE_NO_ERR:
                error = ESP_OK;
                break;
            case MB_MRE_NO_REG:
                error = ESP_ERR_NOT_SUPPORTED;
                break;
            case MB_MRE_TIMEDOUT:
                error = ESP_ERR_TIMEOUT;
                break;
            case MB_MRE_EXE_FUN:
            case MB_MRE_REV_DATA:
                error = ESP_ERR_INVALID_RESPONSE;
                break;
            case MB_MRE_MASTER_BUSY:
                error = ESP_ERR_INVALID_STATE;
                break;
            default:
                ESP_LOGE(TAG, "%s: Incorrect return code (0x%x)", __FUNCTION__, static_cast<int>(mb_error));
                error = ESP_FAIL;
                break;
        }

        (void)xSemaphoreGive(mbm_opts->mbm_sema);

        return error;
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

    bool Core_RS485::get_status()
    {
        return status;
    }
}