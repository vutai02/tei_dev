#pragma once

#include <smooth/core/network/Wifi.h>
#include <smooth/core/Application.h>
#include "common/ObjectModeWifi.h"
#include "utils/DeviceId.h"
#include "Sntp.h"

namespace iotTouch
{
  namespace network
  {
  class WifiAdapter: public smooth::core::ipc::IEventListener<ObjectModeWifi>
  {
    public:
    WifiAdapter(smooth::core::Application& app, iotTouch::DeviceId& id, Sntp& sntp)
      : app_(app), id_(id), sntp_(sntp),
        modeWifi_(ObjectModeWifiQueue::create(2, app_, *this))
    {
    }
    void start();
    void stop();
    void setWifiMode(int mode);
    void setAutoConnect(bool auto_connect);
    void resetConnect();

    std::tuple<bool, std::string, std::string> getConfig();
    int getRssi();
    std::string getMacAddress();

    void event(const ObjectModeWifi & event) override;
    private:
    smooth::core::Application& app_;
    DeviceId& id_;
    Sntp& sntp_;

    using ObjectModeWifiQueue = smooth::core::ipc::SubscribingTaskEventQueue<ObjectModeWifi>;
    std::shared_ptr<ObjectModeWifiQueue> modeWifi_;
    std::shared_ptr<smooth::core::network::Wifi> wifi_{nullptr};
  };
  }
}