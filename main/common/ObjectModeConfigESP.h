#pragma once

#include <string>
#include <smooth/core/json/JsonFile.h>

enum ModeIoTouch {
  AP_MODE = 0,
  SOFTAP_MODE,
  SMARTCONFIG_MODE,
  PROVISIONING_MODE,
  PAIRING_MODE,
  NORMAL_MODE,
  RESTART_SYSTEM_MODE,
  SET_INFO_WIFI_MODE,
  SET_DEVICE_MODE,
  SET_CONFIG_MQTT_MODE
};
class ModeConfigESP
{
  public:
    ModeConfigESP() = default;

    ModeConfigESP(int mode, nlohmann::json json = {})
        : mode(mode),
          json(json)
    {
    }
    int get_mode() const
    {
      return mode;
    }

    const nlohmann::json& get() const
    {
      return json;
    }
  
  private:
    int mode = 0;
    nlohmann::json json = {};
};
