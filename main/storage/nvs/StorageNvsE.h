#pragma once

#include "NvsEsp32.h"

namespace fireAlarm::storage
{
  class StorageNvsE : NVS
  {
    public:
      static StorageNvsE& instance()
      {
        static StorageNvsE cfg{};
        return cfg;
      }

      StorageNvsE& operator=(const StorageNvsE&) = delete;
      StorageNvsE& operator=(StorageNvsE&&) = delete;
      StorageNvsE(const StorageNvsE&) = delete;
      StorageNvsE(StorageNvsE&&) = delete;

      void initialize();

      bool erase_all();
      bool erase_key(const char *key);
  
      bool create(const char *key, const char *value);
      bool create(const char *key, int32_t value);

      bool write(const char *key, const char *value);
      bool write(const char *key, int32_t value);
      bool write(std::string key, std::string value);

      bool read(const char *key, char *dst, uint16_t size);
      bool read(const char *key, int32_t *dst);
      std::string read(std::string key);

    private:
      StorageNvsE();
      void config_default();
  };
}