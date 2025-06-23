#pragma once

#include <smooth/application/network/http/IResponseOperation.h>
#include <smooth/core/network/event/ConnectionStatusEvent.h>
#include <smooth/application/network/http/HTTPProtocol.h>
#include <smooth/core/timer/TimerExpiredEvent.h>
#include <smooth/core/network/NetworkStatus.h>
#include "smooth/core/network/SecureSocket.h"
#include <smooth/core/ipc/IEventListener.h>
#include <smooth/core/ipc/TaskEventQueue.h>
#include <smooth/core/network/Socket.h>
#include <smooth/core/network/IPv4.h>
#include <smooth/core/Application.h>
#include <smooth/core/io/Output.h>
#include <nlohmann/json.hpp>
#include <functional>
#include "network/Ethernet.h"
#include "metroController/MetroController.h"
#include "schedules/SchedulesController.h"
#include "io/IoController.h"
#include "led/LedController.h"
#include "utils/CommandDispatcher.h"
#include "storage/Storage.h"
#include "network/Sntp.h"
#include "network/WifiAdapter.h"
#include "network/Mqtt.h"
#include "utils/DeviceId.h"
#include "ota/Ota.h"
#include "rs485/rs485.h"
namespace fireAlarm
{
  using Proto = smooth::application::network::http::HTTPProtocol;
  using ExpiredQueue = smooth::core::ipc::TaskEventQueue<smooth::core::timer::TimerExpiredEvent>;
  using NetworkStatusQueue =
      smooth::core::ipc::SubscribingTaskEventQueue<smooth::core::network::NetworkStatus>;

  class App : public smooth::core::Application,
              public smooth::core::ipc::IEventListener<smooth::core::network::event::TransmitBufferEmptyEvent>,
              public smooth::core::ipc::IEventListener<smooth::core::network::event::DataAvailableEvent<Proto>>,
              public smooth::core::ipc::IEventListener<smooth::core::network::event::ConnectionStatusEvent>,
              public smooth::core::ipc::IEventListener<smooth::core::network::NetworkStatus>,
              public smooth::application::network::http::IServerResponse
  {
  public:
    App();

    void init() override;

    void tick() override;

    void event(const smooth::core::network::event::TransmitBufferEmptyEvent &) override;

    void event(const smooth::core::network::event::DataAvailableEvent<Proto> &) override;

    void event(const smooth::core::network::event::ConnectionStatusEvent &) override;

    void event(const smooth::core::network::NetworkStatus &ev) override;

    void reply(std::unique_ptr<smooth::application::network::http::IResponseOperation>, bool) override {}

    void reply_error(std::unique_ptr<smooth::application::network::http::IResponseOperation>) override {}

    smooth::core::Task &get_task() override
    {
      return *this;
    }

    void upgrade_to_websocket_internal() override {}

  private:
    void start_mqtt();
    void prepare_config();
    void start_connect_server();
    std::shared_ptr<smooth::core::network::BufferContainer<Proto>> buff_;
#if IS_SSL_REST_API == 0
    std::shared_ptr<smooth::core::network::Socket<Proto>> sock_{};
#else
    std::shared_ptr<smooth::core::network::SecureSocket<Proto>> sock_{};
    std::unique_ptr<smooth::core::network::MBedTLSContext> tls_context{};
#endif
    std::vector<uint8_t> received_content_{};

    std::unique_ptr<Mqtt> mqtt_{};
    fireAlarm::CommandDispatcher cmd_{};

    std::shared_ptr<NetworkStatusQueue> network_status_;

    IoControllerTask io_;
    SchedulesController schedules_;
    fireAlarm::DeviceId id_;
    fireAlarm::network::Sntp sntp_;
    fireAlarm::network::WifiAdapter wifi_;
    Storage storage_;
    fireAlarm::ota::Ota ota_;
    fireAlarm::MetroController metro_;
    led::LedController led_;
    fireAlarm::network::EthAdapter EthAdapter_;

    // fireAlarm::MBTask Master_rs485_;

    fireAlarm::MBTask rs485_; //MASTER
    // fireAlarm::MBTask RS485_;
    void pairActivate();
    void getConfigServer();
    void establishSocketHttp();
    uint8_t stagePairActivate_ = 0;
    uint8_t retry2ServerActivate_ = 0;
    uint8_t cnt_timer_ = 0;
    bool initialize_status_ = false;
    bool connected_ = false;
    uint8_t average_status_wifi_ = 0;
    uint16_t cnt_reset_network_ = 0;
    uint8_t cnt_reset_network_when_activate_ = 0;
    bool is_retry_connect_wifi_ = false;
    void resetConnectWifi();
    uint8_t try_connect_server_ = 0;
    bool is_disconnect_broker_ = false;
  };

}