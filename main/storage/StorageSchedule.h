#pragma once

#include <nlohmann/json.hpp>
#include <smooth/core/json/JsonFile.h>
#include <smooth/core/filesystem/filesystem.h>
#include <smooth/application/security/PasswordHash.h>
#include <smooth/core/filesystem/SPIFlash.h>

namespace fireAlarm::storage
{
  class StorageSchedule
  {
    public:
      static StorageSchedule& instance()
      {
        static StorageSchedule cfg{};
        return cfg;
      }

      StorageSchedule& operator=(const StorageSchedule&) = delete;
      StorageSchedule& operator=(StorageSchedule&&) = delete;
      StorageSchedule(const StorageSchedule&) = delete;
      StorageSchedule(StorageSchedule&&) = delete;

      bool load();
      void save();

      void write_default();
      
      smooth::core::filesystem::Path get_path();

      nlohmann::json& get()
      {
        return f_.value();
      }

    bool isStorageScheduleValid(void);

    private:
      StorageSchedule();
      smooth::core::json::JsonFile f_;
  };
}