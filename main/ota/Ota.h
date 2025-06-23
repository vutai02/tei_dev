#pragma once

#include <memory>
#include <chrono>
#include <smooth/core/Task.h>
#include <smooth/core/Application.h>

#include "utils/Utils.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "esp_http_client.h"
#include "common/ObjectOta.h"
#include "common/SystemHelper.h"
#include "esp_flash_partitions.h"
#include "esp_app_format.h"

namespace fireAlarm::ota
{
  class Ota : 
    public smooth::core::Task,
    public smooth::core::ipc::IEventListener<ObjectOta>
  {
  public:
    Ota();

    void tick() override;
    
    void init() override;

    void event( const ObjectOta& ev) override;

    void stop( const std::string& info = "");
    void startOta();
    void updateStatusOta(uint8_t data);
    esp_http_client_handle_t client_;

    esp_err_t err_;
    std::string url_ota_{};
    bool isAvailable_ = false;
    bool isStartUpdate_ = false;
    int binary_file_length_ = 0;
    esp_ota_handle_t update_handle_ = 0 ;
    /*deal with all receive packet*/
    bool image_header_was_checked_ = false;
    const esp_partition_t *running_ = NULL;
    const esp_partition_t *update_partition_ = NULL;
    fireAlarm::system::SystemHelper helper_;
    fireAlarm::Utils utils_;
  private:
    using ObjectOtaQueue = smooth::core::ipc::SubscribingTaskEventQueue<ObjectOta>;
    std::shared_ptr<ObjectOtaQueue> ev_ota_;
    
    bool diagnostic(void);
    void erase_ota_data(void);
    bool get_running_firmware(void);
    bool get_next_update_partition(void);
    
    pthread_attr_t attribute;
    pthread_t thread;

  };
}