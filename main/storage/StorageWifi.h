#pragma once

#include <nlohmann/json.hpp>
#include <smooth/core/json/JsonFile.h>
#include <smooth/core/filesystem/filesystem.h>
#include <smooth/application/security/PasswordHash.h>
#include <smooth/core/filesystem/SPIFlash.h>

namespace fireAlarm::storage
{
  class StorageWifi
  {
    public:
      static StorageWifi& instance()
      {
        static StorageWifi cfg{};
        return cfg;
      }

      StorageWifi& operator=(const StorageWifi&) = delete;
      StorageWifi& operator=(StorageWifi&&) = delete;
      StorageWifi(const StorageWifi&) = delete;
      StorageWifi(StorageWifi&&) = delete;

      bool load();
      void save();

      smooth::core::filesystem::Path get_path();

      void write_default();

      nlohmann::json& get()
      {
        return f_.value();
      }

      bool isStorageWifiValid(void);
      
    private:
      StorageWifi();
      smooth::core::json::JsonFile f_;
  };
}