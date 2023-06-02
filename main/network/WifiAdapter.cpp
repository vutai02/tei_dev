#include "WifiAdapter.h"

#include <smooth/core/logging/log.h>
#include <smooth/core/util/string_util.h>
#include <smooth/core/json/JsonFile.h>
#include <smooth/core/util/json_util.h>
#include "storage/StorageWifi.h"
#include "common/ConfigConstant.h"
#include "utils/DataCache.hpp"

using namespace iotTouch::storage;
using namespace iotTouch::common;
using namespace smooth::core::json;
using namespace smooth::core::util;
using namespace smooth::core::logging;
using namespace smooth::core::json_util;
using namespace smooth::core::filesystem;

namespace iotTouch
{
  namespace network
  {
    void WifiAdapter::start()
    {
      wifi_ = app_.get_wifi();
      wifi_->set_host_name(id_.get());
      wifi_->set_auto_connect(true);

      int mode = atoi(iotTouch::DataCache::instance().get(WIFI_MODE).c_str());
      setWifiMode(mode);
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
      if (mode == WifiMode::ap_mode) { // ap 
        auto cache_ssid = StorageNvsE::instance().read(SSID);
        auto cache_pass = StorageNvsE::instance().read(KEY);
        if (cache_ssid.length() > 0 && cache_pass.length() > 0) {
          wifi_->set_ap_credentials(
            cache_ssid,
            cache_pass
          );
          wifi_->connect_to_ap();
        }
      } else if (mode == WifiMode::softap_mode) { // softap
        // TODO
      } else if (mode == WifiMode::smartconfig_mode) { // smart config
        wifi_->start_smartconfig();
      } else if (mode == WifiMode::provisioning_mode) {
        wifi_->start_provision();
      }
      else {
        wifi_->connect_to_ap();
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
      return wifi_->get_mac_address();
    }

    void WifiAdapter::event(const ObjectModeWifi &value)
    {
      this->setWifiMode(value.get_mode());
    }
  }
}