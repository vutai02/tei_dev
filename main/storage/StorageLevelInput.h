#pragma once

#include <nlohmann/json.hpp>
#include <smooth/core/json/JsonFile.h>
#include <smooth/core/filesystem/filesystem.h>
#include <smooth/application/security/PasswordHash.h>
#include <smooth/core/filesystem/SPIFlash.h>

namespace fireAlarm::storage
{
    class StorageLevelInput
    {
    public:
        static StorageLevelInput &instance()
        {
            static StorageLevelInput cfg{};
            return cfg;
        }

        StorageLevelInput &operator=(const StorageLevelInput &) = delete;
        StorageLevelInput &operator=(StorageLevelInput &&) = delete;
        StorageLevelInput(const StorageLevelInput &) = delete;
        StorageLevelInput(StorageLevelInput &&) = delete;

        bool load();
        void save();

        void write_default();

        smooth::core::filesystem::Path get_path();

        nlohmann::json &get()
        {
            return f_.value();
        }

        bool isStorageLevelInputValid(void);

    private:
        StorageLevelInput();
        smooth::core::json::JsonFile f_;
    };
}