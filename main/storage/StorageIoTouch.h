#pragma once

#include <nlohmann/json.hpp>
#include <smooth/core/json/JsonFile.h>
#include <smooth/core/filesystem/filesystem.h>
#include <smooth/application/security/PasswordHash.h>
#include <smooth/core/filesystem/SPIFlash.h>

namespace iotTouch::storage
{
  class StorageIoTouch
  {
    public:
      static StorageIoTouch& instance()
      {
        static StorageIoTouch cfg{};
        return cfg;
      }

      StorageIoTouch& operator=(const StorageIoTouch&) = delete;
      StorageIoTouch& operator=(StorageIoTouch&&) = delete;
      StorageIoTouch(const StorageIoTouch&) = delete;
      StorageIoTouch(StorageIoTouch&&) = delete;

      void init();
      bool load();
      void save();
      
      const char* get_path();

      void write_default(bool isWrite = false);

      nlohmann::json& get()
      {
        return f_.value();
      }

    bool isStorageIoTouchValid(void);

    private:
      StorageIoTouch();
      smooth::core::json::JsonFile f_;
  };
}