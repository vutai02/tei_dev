#include <memory>
#include <smooth/core/ipc/Publisher.h>
#include <smooth/core/task_priorities.h>
#include "Ota.h"
#include "esp_sleep.h"
#include "esp32/rom/rtc.h"
#include "freertos/event_groups.h"
#include <esp_system.h>
#include "common/ConstantType.h"
#include "common/ConfigConstant.h"
#include "common/EventLed.hpp"
#include "common/EventPending.hpp"
#include "common/ObjectDataDev2Ser.h"
#include "storage/nvs/StorageNvsE.h"
#include "utils/DataCache.hpp"
#include "spi_flash_mmap.h"
// #include "utils/CaCertPem.h"


using namespace std;
using namespace std::chrono;
using namespace iotTouch::ota;
using namespace iotTouch::common;
using namespace smooth::core::ipc;
using namespace smooth::core::logging;

#define BUFFSIZE 1024


namespace iotTouch::ota
{

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

static constexpr const char* Ota_TAG = "OTA";
static char ota_write_data[BUFFSIZE + 1] = { 0 };

static void http_cleanup(esp_http_client_handle_t client)
{
  esp_http_client_close(client);
  esp_http_client_cleanup(client);
}


Ota::Ota()
  : Task(Ota_TAG, 4 * 1024, smooth::core::APPLICATION_BASE_PRIO, seconds{2}),
    ev_ota_(ObjectOtaQueue::create(2, *this, *this))
{

}

void Ota::tick()
{
}

void Ota::init()
{
  esp_log_level_set(Ota_TAG, static_cast<esp_log_level_t>(TOUCH_OTA_LOGGING_LEVEL));

  get_running_firmware();
  esp_ota_img_states_t ota_state;
  if (esp_ota_get_state_partition(running_, &ota_state) == ESP_OK) {
    if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) {
      // run diagnostic function ...
      bool diagnosticIsOk = diagnostic();
      if (diagnosticIsOk) {
        Log::info(Ota_TAG, "Diagnostics completed successfully! Continuing execution ...");
        esp_ota_mark_app_valid_cancel_rollback();
      } else {
        Log::error(Ota_TAG, "Diagnostics failed! Start rollback to the previous version ...");
        esp_ota_mark_app_invalid_rollback_and_reboot();
      }
    }
  }
  esp_app_desc_t running_app_info;
  if (esp_ota_get_partition_description(running_, &running_app_info) == ESP_OK) {
    iotTouch::DataCache::instance().set(VERSION, running_app_info.version);
    iotTouch::DataCache::instance().set(MODEL, running_app_info.project_name);
    iotTouch::DataCache::instance().set(PRODUCTION_DATE, running_app_info.date);
    iotTouch::DataCache::instance().set(MANUFACTURE, "skytech");
    iotTouch::DataCache::instance().set(CUSTOMER, "skytech");
    iotTouch::DataCache::instance().set(FACTORY, "sieuthuat");
    iotTouch::DataCache::instance().set(ENVIRONMENT, ENV);
  }
}

bool Ota::diagnostic()
{
  return true;
}

void Ota::stop( const std::string& info )
{
  isAvailable_ = false;
  isStartUpdate_ = false;
  Publisher<led::EventLed>::publish(
    led::EventLed("exit_ota")
  );

  Publisher<EventPending>::publish(
    EventPending(false, info)
  );

  StorageNvsE::instance().erase_key("link_ota");

  helper_.restartSystem();

  pthread_cancel(thread);
}

static void* sc(void* arg)
{
  auto ota = static_cast<Ota*>(arg);
  if (ota->isAvailable_) {
    while(ota->isStartUpdate_) {
      int data_read = esp_http_client_read(ota->client_, ota_write_data, BUFFSIZE);
      if (data_read < 0) {
        Log::error(Ota_TAG, "Error: SSL data read error\n");
        http_cleanup(ota->client_);
        ota->stop("Error: SSL data read error");
      } else if (data_read > 0) {
        if (ota->image_header_was_checked_ == false) {
          esp_app_desc_t new_app_info;
          if (data_read > sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t) + sizeof(esp_app_desc_t)) {
            // check current version with downloading
            memcpy(&new_app_info, &ota_write_data[sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t)], sizeof(esp_app_desc_t));
            printf("[OTA]New firmware version: %s\n", new_app_info.version);

            esp_app_desc_t running_app_info;
            if (esp_ota_get_partition_description(ota->running_, &running_app_info) == ESP_OK) {
              printf("[OTA]Running firmware version: %s\n", running_app_info.version);
            }

            const esp_partition_t* last_invalid_app = esp_ota_get_last_invalid_partition();
            esp_app_desc_t invalid_app_info;
            if (esp_ota_get_partition_description(last_invalid_app, &invalid_app_info) == ESP_OK) {
              printf("[OTA]Last invalid firmware version: %s\n", invalid_app_info.version);
            }

            // check current version with last invalid partition
            if (last_invalid_app != NULL) {
              if (memcmp(invalid_app_info.version, new_app_info.version, sizeof(new_app_info.version)) == 0) {
                printf("[OTA][warning]New version is the same as invalid version.\n");
                printf("[OTA][warning]Previously, there was an attempt to launch the firmware with %s version, but it failed.\n", invalid_app_info.version);
                printf("[OTA][warning]The firmware has been rolled back to the previous version.\n");
                http_cleanup(ota->client_);
                ota->stop("New version is the same as invalid version.");
                break;
              }
            }

            if (memcmp(new_app_info.version, running_app_info.version, sizeof(new_app_info.version)) == 0) {
              printf("[OTA][warning]Current running version is the same as a new. We will not continue the update.\n");
              http_cleanup(ota->client_);
              ota->stop("Current running version is the same as a new. We will not continue the update.");
              break;
            }

            if (memcmp(new_app_info.project_name, running_app_info.project_name, sizeof(new_app_info.project_name)) != 0) {
              printf("[OTA][warning] project name failed. %s, %s \n", new_app_info.project_name, running_app_info.project_name);
              http_cleanup(ota->client_);
              ota->stop("Project name failed");
              break;
            }

            int current_version = ota->utils_.getVersion(running_app_info.version);
            int new_version = ota->utils_.getVersion(new_app_info.version);
            if (current_version > new_version) {
              printf("[OTA][warning]Current running version is the bigger as a new. We will not continue the update.\n");
              http_cleanup(ota->client_);
              ota->stop("Current running version is the bigger as a new. We will not continue the update.");
              break;
            }

            ota->image_header_was_checked_ = true;

            ota->err_ = esp_ota_begin(ota->update_partition_, OTA_SIZE_UNKNOWN, &ota->update_handle_);
            if (ota->err_ != ESP_OK) {
              printf("[OTA][error]esp_ota_begin failed (%s)\n", esp_err_to_name(ota->err_));
              http_cleanup(ota->client_);
              ota->stop("esp_ota_begin failed");
              break;
            }
          
            printf("[OTA][info]esp_ota_begin succeeded\n");
          } else {
            printf("[OTA][error]received package is not fit len\n");
            http_cleanup(ota->client_);
            ota->stop("received package is not fit len");
            break;
          }
        }
        ota->err_ = esp_ota_write( ota->update_handle_, (const void *)ota_write_data, data_read);
        if (ota->err_ != ESP_OK) {
          http_cleanup(ota->client_);
          ota->stop("Write image to flash failed");
        }

        std::this_thread::sleep_for(std::chrono::milliseconds{ 5 });

        ota->binary_file_length_ += data_read;
        printf("[OTA][info]Written image length %d\n", ota->binary_file_length_);
      } else if (data_read == 0) {
        if (esp_http_client_is_complete_data_received(ota->client_) == true) {
          printf("[OTA][info]Connection closed,all data received\n");
          ota->isStartUpdate_ = false;
          break;
        }
      }
    }
    
    if (ota->isAvailable_) {
      printf("[OTA][info]Total Write binary data length: %d\n", ota->binary_file_length_);
      if (ota->binary_file_length_) {
        if (esp_http_client_is_complete_data_received(ota->client_) != true) {
          printf("[OTA][error]Error in receiving complete file\n");
          http_cleanup(ota->client_);
          ota->stop("Error in receiving complete file");
        }

        ota->err_ = esp_ota_end(ota->update_handle_);
        if (ota->err_ != ESP_OK) {
          if (ota->err_ == ESP_ERR_OTA_VALIDATE_FAILED) {
            printf("[OTA][error]Image validation failed, image is corrupted\n");
          }
          printf("[OTA][error]esp_ota_end failed (%s)!\n", esp_err_to_name(ota->err_));
          http_cleanup(ota->client_);
          ota->stop("esp_ota_end failed");
        }

        ota->err_ = esp_ota_set_boot_partition(ota->update_partition_);
        if (ota->err_ != ESP_OK) {
          printf("[OTA][error]esp_ota_set_boot_partition failed (%s)!\n", esp_err_to_name(ota->err_));
          http_cleanup(ota->client_);
          ota->stop("esp_ota_set_boot_partition failed");
        }
        else {
          printf("[OTA][info] Prepare to restart system!\n");

          nlohmann::json v;
          v["status"] = 1;
          v["desc"] = "Update firmware successfully";

          Publisher<ObjectDataDev2Ser>::publish(
            ObjectDataDev2Ser(
              TYPE_EXTERNAL_OTA,
              "ack",
              v,
              "0000000"
            )
          );

          StorageNvsE::instance().erase_key("link_ota");

          std::this_thread::sleep_for(std::chrono::seconds{ 5 });

          ota->helper_.restartChip();
        }
      }
      else {
        std::string length_failed = "binary_file_length failed: " + std::to_string(ota->binary_file_length_);
        ota->stop(length_failed);
      }
    }
  }
  return NULL;
}

void Ota::startOta()
{
  int err_thread;
  esp_err_t err;
  esp_http_client_config_t config = {
    .url = url_ota_.c_str(),
    // .cert_pem = (char *)ca_cert_pem_start,
    .timeout_ms = 20000,
  };

  client_ = esp_http_client_init(&config);
  if (client_ == NULL) {
    Log::info(Ota_TAG, "Failed to initialise HTTP connection");
    return;
  }

  err = esp_http_client_open(client_, 0);
  if (err != ESP_OK) {
    Log::info(Ota_TAG, "Failed to open HTTP connection: {}", esp_err_to_name(err));
    esp_http_client_cleanup(client_);
    stop("Failed to open HTTP connection");
    return;
  }
  esp_http_client_fetch_headers(client_);

  if (get_next_update_partition()) {
    Log::info(Ota_TAG,"oke start udpate");
    isStartUpdate_ = true;
  }

  err_thread = pthread_attr_init(&attribute);
  if (err_thread) {
    stop();
    return;
  }
  err_thread = pthread_attr_setstacksize(&attribute, 1024 * 4);
  if (err_thread) {
    stop();
    return;
  }
  err_thread = pthread_create(&thread,&attribute,sc,this);
  if (err_thread) {
    stop();
    return;
  }
  err_thread = pthread_detach(thread);
  if (err_thread) {
    stop();
    return;
  }
}


void Ota::event(const ObjectOta& ev)
{
  if (!isStartUpdate_ && !isAvailable_) {
    isAvailable_ = ev.get_mode();
    url_ota_ = ev.get_url();
    Log::info(Ota_TAG,"link OTA: {}", url_ota_);
    Publisher<led::EventLed>::publish(
      led::EventLed("entry_ota")
    );

    Publisher<EventPending>::publish(
      EventPending(true, "started update firmware")
    );

    startOta();
  }
  else {
    Publisher<EventPending>::publish(
      EventPending(true, "Please waitting update firmware")
    );
  }
}

bool Ota::get_running_firmware()
{
  const esp_partition_t *configured = esp_ota_get_boot_partition();
  running_ = esp_ota_get_running_partition();
  Log::info(Ota_TAG, "Running partition type {0} subtype {1} (offset 0x{2:08x}) label: {3}",
      running_->type, running_->subtype, running_->address, running_->label);
  Log::info(Ota_TAG, "Configured partition type {0} subtype {1} (offset 0x{2:08x}) label: {3}",
      configured->type, configured->subtype, configured->address, configured->label);

  if (configured != running_) {
    Log::info(Ota_TAG, "Configured OTA boot partition at offset 0x{0}, but running from offset 0x{1:x}",
      configured->address, running_->address);
    Log::info(Ota_TAG, "(This can happen if either the OTA boot data or preferred boot image become corrupted somehow.)");
  }

  Log::info(Ota_TAG, "Running partition type {0} subtype {1} (offset 0x{2:x})",
    running_->type, running_->subtype, running_->address);
  if (running_ != NULL) return true;
  else return false;
}

bool Ota::get_next_update_partition(void)
{
  update_partition_ = esp_ota_get_next_update_partition(NULL);
  Log::info(Ota_TAG, "Writing to partition subtype {0} at offset 0x{1:x}",
      update_partition_->subtype, update_partition_->address);
  if (update_partition_ != NULL) return true;
  else return false;
}

void Ota::erase_ota_data(void)
{
  const esp_partition_t *data_partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_OTA, NULL);
  if (data_partition != NULL) {
    esp_partition_erase_range(data_partition, 0, 2 * SPI_FLASH_SEC_SIZE);
  }
}

#pragma GCC diagnostic pop
}