#pragma once

#include <nlohmann/json.hpp>
#include <smooth/core/json/JsonFile.h>
#include <smooth/core/filesystem/filesystem.h>
#include <smooth/application/security/PasswordHash.h>
#include <smooth/core/filesystem/SPIFlash.h>

namespace fireAlarm::storage
{
  class StorageInfoDevice
  {
    public:
      static StorageInfoDevice& instance()
      {
        static StorageInfoDevice cfg{};
        return cfg;
      }

      StorageInfoDevice& operator=(const StorageInfoDevice&) = delete;
      StorageInfoDevice& operator=(StorageInfoDevice&&) = delete;
      StorageInfoDevice(const StorageInfoDevice&) = delete;
      StorageInfoDevice(StorageInfoDevice&&) = delete;

      bool load();
      void save();

      smooth::core::filesystem::Path get_path();
      
      void write_default();

      nlohmann::json& get()
      {
        return f_.value();
      }

    bool isStorageInfoDeviceValid(void);

    private:
      StorageInfoDevice();
      smooth::core::json::JsonFile f_;
  };
}