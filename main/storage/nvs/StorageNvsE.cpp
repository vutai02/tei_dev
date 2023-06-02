#include "StorageNvsE.h"
#include "common/ConfigConstant.h"

namespace iotTouch::storage
{
  using namespace iotTouch::common;

  StorageNvsE::StorageNvsE(){}

  void StorageNvsE::initialize()
  {
    init("nvs", "storage_");

    config_default();
  }
  
  bool StorageNvsE::erase_all()
  {
    return erase_all_nvs() ? true : false;
  }

  bool StorageNvsE::erase_key(const char *key)
  {
    return erase_key_nvs(key) ? true : false;
  }
  
  bool StorageNvsE::create(const char *key, const char *value)
  {
    return create_nvs(key, value) ? true : false;
  }

  bool StorageNvsE::create(const char *key, int32_t value)
  {
    return create_nvs(key, value) ? true : false;
  }


  bool StorageNvsE::write(const char *key, const char *value)
  {
    return write_nvs(key, value) ? true : false;
  }

  bool StorageNvsE::write(const char *key, int32_t value)
  {
    return write_nvs(key, value) ? true : false;
  }

  bool StorageNvsE::write(std::string key, std::string value)
  {
    return write_nvs(key, value) ? true : false;
  }

  bool StorageNvsE::read(const char *key, char *dst, uint16_t size)
  {
    return read_nvs(key, dst, size) ? true : false;
  }

  bool StorageNvsE::read(const char *key, int32_t *dst)
  {
    return read_nvs(key, dst) ? true : false;
  }

  std::string StorageNvsE::read(std::string key)
  {
    return read_nvs(key);
  }

  void StorageNvsE::config_default()
  {
    int32_t value_default = 0;
    std::string id_sys_ = "";

#if TOUCH_TYPE  >= 1
    if (!read_nvs("touch_0", &value_default)) {
      create_nvs("touch_0", value_default);
    }
    if (!read_nvs("cnt_touch_0", &value_default)) {
      create_nvs("cnt_touch_0", value_default);
    }
#if TOUCH_TYPE  >= 2
    if (!read_nvs("touch_1", &value_default)) {
      create_nvs("touch_1", value_default);
    }
    if (!read_nvs("cnt_touch_1", &value_default)) {
      create_nvs("cnt_touch_1", value_default);
    }

#if TOUCH_TYPE  >= 3
    if (!read_nvs("touch_2", &value_default)) {
      create_nvs("touch_2", value_default);
    }
    if (!read_nvs("cnt_touch_2", &value_default)) {
      create_nvs("cnt_touch_2", value_default);
    }
#if TOUCH_TYPE  >= 4
    if (!read_nvs("touch_3", &value_default)) {
      create_nvs("touch_3", value_default);
    }
    if (!read_nvs("cnt_touch_3", &value_default)) {
      create_nvs("cnt_touch_3", value_default);
    }
#endif
#endif
#endif
#endif
    
    if (!read_nvs("id_sys", id_sys_)) {
      create_nvs("id_sys", "0");
    }

    if (!read_nvs(WIFI_MODE, &value_default)) {
      create_nvs(WIFI_MODE, value_default);
    }
  }
}