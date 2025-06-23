#include "WifiAdapter.h"

#include <smooth/core/logging/log.h>
#include <smooth/core/util/string_util.h>
#include <smooth/core/json/JsonFile.h>
#include <smooth/core/util/json_util.h>
#include "storage/StorageWifi.h"
#include "common/ConfigConstant.h"
#include "utils/DataCache.hpp"

using namespace fireAlarm::storage;
using namespace fireAlarm::common;
using namespace smooth::core::json;
using namespace smooth::core::util;
using namespace smooth::core::logging;
using namespace smooth::core::json_util;
using namespace smooth::core::filesystem;

namespace fireAlarm
{
  namespace network
  {
    void WifiAdapter::start()
    {
      wifi_ = app_.get_wifi();
      wifi_->set_host_name(id_.get());
      wifi_->set_auto_connect(true);
      Log::info("WifiAdapter", "Wi-Fi started with hostname: {}", id_.get());

      int mode = atoi(fireAlarm::DataCache::instance().get(WIFI_MODE).c_str());
      setWifiMode(0);
      Log::info("WifiAdapter", "Wi-Fi Mode: {}", 0);
      sntp_.start();
    }

    void WifiAdapter::stop()
    {
      wifi_->stop();
    }
    
    void WifiAdapter::setAutoConnect(bool auto_connect)
    {
      wifi_->set_auto_connect(auto_connect);
    }

    void WifiAdapter::resetConnect()
    {
      wifi_->reset_connect();
    }

    void WifiAdapter::setWifiMode(int mode)
    {
      if (mode == WifiMode::ap_mode) {
        // auto cache_ssid = StorageNvsE::instance().read(SSID);
        // auto cache_pass = StorageNvsE::instance().read(KEY);
        std::string cache_ssid = "47K1";
        std::string cache_pass = "99999999";
        // Log::info("WifiAdapter", "AP Mode: SSID: {}, Password: {}", cache_ssid, cache_pass);
        if (cache_ssid.length() > 0 && cache_pass.length() > 0) {
          wifi_->set_ap_credentials(
            cache_ssid,
            cache_pass
          );
          wifi_->connect_to_ap();
          Log::info("WifiAdapter", "Connecting to AP with SSID: {}", cache_ssid, cache_pass);
        }
      } else if (mode == WifiMode::softap_mode) { // softap
      //   // TODO
      } else if (mode == WifiMode::smartconfig_mode) { // smart config
        wifi_->start_smartconfig();
      } else if (mode == WifiMode::provisioning_mode) {
        wifi_->start_provision();
      }
      else if(mode == WifiMode::change_ssid_pass) {
        auto cache_ssid = StorageNvsE::instance().read(SSID);
        auto cache_key = StorageNvsE::instance().read(KEY);
        Log::info("WifiAdapter", "Change SSID/Password: SSID: {}, Password: {}", cache_ssid, cache_pass);
        if (cache_ssid.length() > 0 && cache_key.length() > 0) {
          wifi_->set_ap_credentials(
            cache_ssid,
            cache_key
          );
          wifi_->connect_to_ap();
          Log::info("WifiAdapter", "Connecting to AP with SSID: {}", cache_ssid, cache_pass);
        }
        else {
          wifi_->connect_to_ap();
        }
      }
    }

    std::tuple<bool, std::string, std::string> WifiAdapter::getConfig()
    {
      return wifi_->get_config();
    }

    int WifiAdapter::getRssi()
    {
      return wifi_->get_rssi();
    }

    std::string WifiAdapter::getMacAddress()
    {
      Log::info("WifiAdapter", "Device MAC Address: {}",  wifi_->get_mac_address());
      return wifi_->get_mac_address();
    }

    void WifiAdapter::event(const ObjectModeWifi &value)
    {
      this->setWifiMode(value.get_mode());
    }
  }
}