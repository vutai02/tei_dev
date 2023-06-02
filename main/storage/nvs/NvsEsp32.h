#pragma once

#include <esp_err.h>
#include <esp_log.h>

#include <string>
#include <vector>

extern "C" {
#include "esp_partition.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "nvs.h"
}


class NVS
{
	private:
		const char tag[4] = "NVS";
		nvs_handle _handler;
		int8_t _debug = 1;

	public:
		int8_t init(const char *partition, const char *name, int8_t debug = 0);
		int8_t erase_all_nvs();
		int8_t erase_key_nvs(const char *key);

		int8_t create_nvs(const char *key, const char *value);
		int8_t create_nvs(const char *key, int32_t value);

		int8_t write_nvs(const char *key, const char *value);
		int8_t write_nvs(const char *key, int32_t value);
		bool  write_nvs(std::string key, std::string value);


		int8_t read_nvs(const char *key, char *dst, uint16_t size);
		int8_t read_nvs(const char *key, int32_t *dst);
		bool   read_nvs(std::string key, std::string& res);
		std::string  read_nvs(std::string key);
		
};