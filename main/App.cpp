#include "App.h"
#include <smooth/application/network/http/regular/ResponseCodes.h>
#include <smooth/application/network/http/regular/HTTPMethod.h>
#include <smooth/application/network/http/HTTPPacket.h>
#include <smooth/core/ipc/SubscribingTaskEventQueue.h>
#include <smooth/core/network/BufferContainer.h>
#include <smooth/core/network/MbedTLSContext.h>
#include <smooth/application/hash/base64.h>
#include <smooth/core/SystemStatistics.h>
#include <smooth/core/task_priorities.h>
#include <smooth/core/util/json_util.h>
#include <smooth/core/json/JsonFile.h>
#include <smooth/core/ipc/Publisher.h>
#include <smooth/core/network/Wifi.h>
#include <smooth/core/logging/log.h>
#include "common/digital/SensitivityTouchValue.h"
#include "common/TriggerUpdateStorage.h"
#include "common/ObjectModeConfigESP.h"
#include "common/ObjectUpdateStatus.h"
#include "common/ObjectSetCountDown.h"
#include "storage/nvs/StorageNvsE.h"
#include "common/ObjectSetTimer.h"
#include "common/ConfigConstant.h"
#include "common/ConstantType.h"
#include "common/ObjectOta.h"
#include "utils/CaCertPem.h"
#include "utils/timer_id.h"
#include "utils/DataCache.hpp"
#include "storage/StorageInfoDevice.h"

using namespace fireAlarm::storage;

using namespace std::chrono;
using namespace smooth::core;
using namespace fireAlarm::common;
using namespace smooth::core::ipc;
using namespace smooth::core::json;
using namespace smooth::core::network;
using namespace smooth::core::logging;
using namespace smooth::core::json_util;
using namespace smooth::core::filesystem;
using namespace smooth::core::network::event;
using namespace smooth::application::network::http;
using namespace smooth::application::hash::base64;

namespace fireAlarm
{
  static constexpr const char *App_TAG = "App";
  constexpr const int max_timer_ping_server = 30;

  App::App()
      : Application(smooth::core::APPLICATION_BASE_PRIO, seconds{1}),
        network_status_(NetworkStatusQueue::create(3, *this, *this)),
        io_(),
        schedules_(),
        id_(),
        sntp_(*this),
        wifi_(*this, id_, sntp_),
        storage_(App_TAG, *this),
        ota_(),
        metro_(),
        led_(*this),
        EthAdapter_()
        // Master_rs485_()
        // rs485_() //MASTER
        // RS485_()
  {
  }

  void App::init()
  {
    esp_log_level_set(
        App_TAG,
        static_cast<esp_log_level_t>(TOUCH_LOGGING_LEVEL));

    Application::init();
    io_.start();
    schedules_.start();
    // EthAdapter_.start();
    wifi_.start();
    //  ota_.start();
    metro_.start();
    // Master_rs485_.start();
    // rs485_.start(); //MASTER
    // RS485_.start();
    
    wifi_.setWifiMode(0);
    // if (fireAlarm::DataCache::instance().get(WIFI_MODE) == std::to_string(WifiMode::smartconfig_mode))
    // {
    //   Publisher<led::EventLed>::publish(
    //       led::EventLed("smart"));
    // }
    // if (fireAlarm::DataCache::instance().get(WIFI_MODE) == std::to_string(WifiMode::provisioning_mode))
    // {
    //   Publisher<led::EventLed>::publish(
    //       led::EventLed("provisioning"));
    // }

    received_content_.clear();
  }

  void App::tick()
  {
    Log::info(App_TAG, "[APP] Free memory: {} bytes", esp_get_free_heap_size());

    // SystemStatistics::instance().dump();
    fireAlarm::DataCache::instance().set("rssi", std::to_string(wifi_.getRssi()));

    if (!initialize_status_)
    {
      if (mqtt_ && mqtt_->is_connected())
      {
        metro_.ping();
        metro_.updateStatus();

        Publisher<ObjectDataSendToMetro>::publish(ObjectDataSendToMetro(
            TYPE_INTERNALL_TOUCH_SETTING,
            "",
            {}));

        initialize_status_ = true;
      }
    }

    resetConnectWifi();

    if (mqtt_ && mqtt_->get_cnt_reconnect_broker() >= 10)
    {
      fireAlarm::DataCache::instance().set(ACTIVATE, "0");
      mqtt_->reset_cnt_reconnect_broker();
      mqtt_->disconnect_broker();
      stagePairActivate_ = 1;

      if (sock_)
        sock_.reset();
      establishSocketHttp();
    }
  }

  void App::resetConnectWifi()
  {
    if (fireAlarm::DataCache::instance().get(ACTIVATE) == "1")
    {
      if (is_retry_connect_wifi_)
      {
        cnt_reset_network_++;
        if (cnt_reset_network_ >= 300)
        {
          cnt_reset_network_ = 0;
          wifi_.setAutoConnect(true);
          wifi_.resetConnect();
          is_retry_connect_wifi_ = false;

          Publisher<SensitivityTouchValue>::publish(
              SensitivityTouchValue(
                  TOUCH_TYPE,
                  1));
        }
      }
    }
  }

  void App::event(const smooth::core::network::NetworkStatus &ev)
  {
    connected_ = ev.get_event() == NetworkEvent::GOT_IP;
    Log::info(App_TAG, "status network: {}", connected_);

    if (connected_)
    {
      average_status_wifi_ = 0;
      is_disconnect_broker_ = false;

      if (fireAlarm::DataCache::instance().get(ACTIVATE) == "0")
      {
        // activate
        auto [isExisting, ssid, password] = wifi_.getConfig();
        if (isExisting && ssid.length() > 0 && password.length() > 0)
        {

          fireAlarm::DataCache::instance().set(SSID, ssid);
          fireAlarm::DataCache::instance().set(KEY, password);
          fireAlarm::DataCache::instance().set(MAC_ADDR, wifi_.getMacAddress());

          Publisher<led::EventLed>::publish(
              led::EventLed("receive_data"));
          establishSocketHttp();
        }
      }
      else
      {
        auto userMqtt = fireAlarm::DataCache::instance().get(USER);
        auto passMqtt = fireAlarm::DataCache::instance().get(PASS);
        auto link_ota = StorageNvsE::instance().read("link_ota");
        if (!link_ota.empty())
        {
          Publisher<ObjectOta>::publish(ObjectOta(
              true,
              link_ota));
        }
        else
        {
          if (userMqtt.length() && passMqtt.length())
          {
            start_mqtt();
          }
        }
      }
    }
    else
    {
      average_status_wifi_++;
      if (!is_disconnect_broker_)
      {
        if (mqtt_)
          mqtt_->disconnect_broker();
        is_disconnect_broker_ = true;
      }
    }

    if (fireAlarm::DataCache::instance().get(WIFI_MODE) != std::to_string(WifiMode::ap_mode))
    {

      cnt_reset_network_when_activate_++;
      if (cnt_reset_network_when_activate_ >= 20)
      {
        Publisher<ObjectDataSendToMetro>::publish(
            ObjectDataSendToMetro(
                TYPE_INTERNAL_RESET,
                "",
                {}));
        cnt_reset_network_when_activate_ = 0;
      }
    }

    if (average_status_wifi_ >= 15)
    {
      average_status_wifi_ = 0;
      is_retry_connect_wifi_ = true;
      wifi_.setAutoConnect(false);
      wifi_.stop();

      Publisher<SensitivityTouchValue>::publish(
          SensitivityTouchValue(
              TOUCH_TYPE,
              TOUCH_BUTTON_MAX_CHANGE_RATE_HOT));
    }
  }

  void App::event(const TransmitBufferEmptyEvent &)
  {
  }

  void App::event(const DataAvailableEvent<Proto> &packet)
  {
    Proto::packet_type p;
    packet.get(p);

    if (!p.is_continuation())
    {
      // First packet
      Log::info(App_TAG, "response code: {}", p.response_code());
      received_content_.insert(
          received_content_.end(),
          p.data().begin(),
          p.data().end());
    }
    else
    {
      // Seconds and onwards
      received_content_.insert(
          received_content_.end(),
          p.data().begin(),
          p.data().end());
    }

    if (!p.is_continued())
    {
      // Last packet
      if (sock_)
      {
        sock_->stop("Last packet received");
      }

      std::stringstream ss;
      for (auto c : received_content_)
      {
        ss << static_cast<char>(c);
      }

      if (received_content_.size() > 0)
      {

        std::string s{ss.str()};
        Log::info(App_TAG, "res: {}", s);
        Log::info(App_TAG, "response code: {}", p.response_code());
        try
        {
          nlohmann::json bootstrap = nlohmann::json::parse(ss.str());

          if (default_value(bootstrap, "success", false))
          {

            if (default_value(bootstrap["result"], EXTERNAL_ID, "") != "")
            {
              stagePairActivate_++;

              fireAlarm::DataCache::instance().set(EXTERNAL_ID,
                                                  default_value(bootstrap["result"], EXTERNAL_ID, ""));
              fireAlarm::DataCache::instance().set(EXTERNAL_KEY,
                                                  default_value(bootstrap["result"], EXTERNAL_KEY, ""));

              start_connect_server();
            }
            else if (bootstrap.contains("result") && bootstrap["result"].is_string())
            {
              fireAlarm::DataCache::instance().set(IP, default_value(bootstrap, "result", ""));
              if (buff_)
                buff_.reset();
#if IS_SSL_REST_API == 1
              if (tls_context)
                tls_context.reset();
#endif
              if (sock_)
                sock_.reset();
            }
            else
            {
              if (bootstrap.contains("result") && bootstrap["result"].is_object() && bootstrap["result"].contains("thingId"))
              {

                std::string user = default_value(bootstrap["result"], "thingId", "");
                std::string pass = default_value(bootstrap["result"], "thingKey", "");
                std::string client_id = default_value(bootstrap["result"], "deviceId", "");
                std::string scheme = default_value(bootstrap["result"], "scheme", "mqtt");
                std::string mqtt_endpoint = default_value(bootstrap["result"], "ip", DEFAULT_ENDPOINT_CLOUD);
                int mqtt_port = default_value(bootstrap["result"], "port", 1883);

                fireAlarm::DataCache::instance().set(USER, user);
                fireAlarm::DataCache::instance().set(PASS, pass);
                fireAlarm::DataCache::instance().set(CLIENT_ID, client_id);
                fireAlarm::DataCache::instance().set(MQTT_ENDPOINT, mqtt_endpoint);
                fireAlarm::DataCache::instance().set(MQTT_PORT, std::to_string(mqtt_port));
                fireAlarm::DataCache::instance().set(SCHEME, scheme);

                fireAlarm::DataCache::instance().set(ACTIVATE, "1");
                fireAlarm::DataCache::instance().set(WIFI_MODE, std::to_string(WifiMode::ap_mode));

                for (const auto &channel : bootstrap["result"]["channels"])
                {
                  auto channelName = channel[NAME].get<std::string>();
                  std::string::size_type pos = channelName.find("_");
                  auto name = channelName.substr(0, pos);
                  fireAlarm::DataCache::instance().set(name, channel[ID]);
                }

                Publisher<TriggerUpdateStorage>::publish(
                    TriggerUpdateStorage(true));

                Publisher<led::EventLed>::publish(
                    led::EventLed("activated"));

                if (buff_)
                  buff_.reset();
#if IS_SSL_REST_API == 1
                if (tls_context)
                  tls_context.reset();
#endif
                if (sock_)
                  sock_.reset();

                std::this_thread::sleep_for(std::chrono::milliseconds{5000});
                start_mqtt();
              }
            }
            Log::info(App_TAG, "============> SUCCESS <============ ");
          }
          else
          {
            Log::error(App_TAG, "============> API ERROR <============ ");

            Publisher<led::EventLed>::publish(
                led::EventLed("error"));
          }
        }
        catch (const std::exception &e)
        {
          Publisher<led::EventLed>::publish(
              led::EventLed("error"));
          std::cerr << e.what() << '\n';
        }
      }

      received_content_.clear();
    }
  }

  void App::start_mqtt()
  {
    if (!mqtt_)
    {
      auto client_id = fireAlarm::DataCache::instance().get(CLIENT_ID);
      mqtt_ = std::make_unique<Mqtt>(client_id, *this, cmd_);

      /* Setup subscriptions control */
      {
        static std::string channelsControl = "channels/";
        channelsControl.append(fireAlarm::DataCache::instance().get("control"));
        channelsControl.append("/messages");

        mqtt_->add_subscription(channelsControl);

        cmd_.add_command(
            TYPE_EXTERNAL_CONTROL_TOUCH,
            [this](const std::string &command, const std::string &data, const std::string &id_package)
            {
              try
              {
                nlohmann::json json_ = nlohmann::json::parse(data);
                auto id1 = fireAlarm::DataCache::instance().get(USER);
                auto id2 = default_value(json_, ID, "");
                if (id1 == id2)
                {
                  auto payload = json_[DATA];
                  // v[DATA1] = true;
                  // DATA : { true
                  // }
                  if (payload.is_object())
                  {

                    Publisher<ObjectDataSendToMetro>::publish(ObjectDataSendToMetro(
                        command,
                        default_value(json_, ACTION, ""),
                        payload,
                        id_package));
                  }
                }
              }
              catch (...)
              {
              }
            });

        cmd_.add_command(
            TYPE_EXTERNAL_SET_SCHEDULE,
            [this](const std::string &command, const std::string &data, const std::string &id_package)
            {
              try
              {
                nlohmann::json json_ = nlohmann::json::parse(data);
                auto id1 = fireAlarm::DataCache::instance().get(USER);
                auto id2 = default_value(json_, ID, "");
                if (id1 == id2)
                {
                  auto payload = json_[DATA];
                  if (payload.is_object())
                  {

                    Publisher<ObjectDataSendToMetro>::publish(ObjectDataSendToMetro(
                        command,
                        default_value(json_, ACTION, ""),
                        payload,
                        id_package));
                  }
                }
              }
              catch (...)
              {
              }
            });

        cmd_.add_command(
            TYPE_EXTERNAL_SET_COUNTDOWN,
            [this](const std::string &command, const std::string &data, const std::string &id_package)
            {
              try
              {
                nlohmann::json json_ = nlohmann::json::parse(data);
                auto id1 = fireAlarm::DataCache::instance().get(USER);
                auto id2 = default_value(json_, ID, "");
                if (id1 == id2)
                {
                  auto payload = json_[DATA];
                  if (payload.is_object())
                  {

                    Publisher<ObjectDataSendToMetro>::publish(ObjectDataSendToMetro(
                        command,
                        default_value(json_, ACTION, ""),
                        payload,
                        id_package));
                  }
                }
              }
              catch (...)
              {
              }
            });

        cmd_.add_command(
            TYPE_EXTERNAL_OTA,
            [this](const std::string &command, const std::string &data, const std::string &id_package)
            {
              try
              {
                nlohmann::json json_ = nlohmann::json::parse(data);
                Publisher<ObjectDataSendToMetro>::publish(ObjectDataSendToMetro(
                    command,
                    default_value(json_, ACTION, ""),
                    json_,
                    id_package));
              }
              catch (...)
              {
              }
            });

        cmd_.add_command(
            TYPE_EXTERNAL_SET_SLEEP_LED_RED,
            [this](const std::string &command, const std::string &data, const std::string &id_package)
            {
              try
              {
                nlohmann::json json_ = nlohmann::json::parse(data);
                auto id1 = fireAlarm::DataCache::instance().get(USER);
                auto id2 = default_value(json_, ID, "");
                if (id1 == id2)
                {
                  auto payload = json_[DATA];
                  if (payload.is_object())
                  {
                    Publisher<ObjectDataSendToMetro>::publish(ObjectDataSendToMetro(
                        command,
                        default_value(json_, ACTION, ""),
                        payload,
                        id_package));
                  }
                }
              }
              catch (...)
              {
              }
            });
      }

      mqtt_->start();
    }
    else
    {
      mqtt_->set_authorization(
          fireAlarm::DataCache::instance().get(USER),
          fireAlarm::DataCache::instance().get(PASS));

      static std::string channelsConfig = "channels/";
      channelsConfig.append(fireAlarm::DataCache::instance().get("config"));
      channelsConfig.append("/messages");

      mqtt_->add_subscription(channelsConfig);
      static std::string channelsControl = "channels/";
      channelsControl.append(fireAlarm::DataCache::instance().get("control"));
      channelsControl.append("/messages");

      mqtt_->add_subscription(channelsControl);

      mqtt_->subscription();

      initialize_status_ = false;

      std::string broker = fireAlarm::DataCache::instance().get(MQTT_ENDPOINT);
      int port = stoi(fireAlarm::DataCache::instance().get(MQTT_PORT));
      mqtt_->connect_to(broker, port);
    }
  }

  void App::event(const ConnectionStatusEvent &ev)
  {
    Log::info(App_TAG, "status connect to server: {}", ev.is_connected());
    if (ev.is_connected())
    {
      if (fireAlarm::DataCache::instance().get(ACTIVATE) == "0")
      {
        if (!stagePairActivate_)
        {
          pairActivate();
        }
        else
        {
          getConfigServer();
        }
      }
    }
    else
    {
      // retry connect 2 server
      if (fireAlarm::DataCache::instance().get(ACTIVATE) == "0")
      {
        retry2ServerActivate_++;
        std::this_thread::sleep_for(std::chrono::milliseconds{1000});
        start_connect_server();
      }

      if (retry2ServerActivate_ >= 4)
      {
        Publisher<ObjectDataSendToMetro>::publish(ObjectDataSendToMetro(
            TYPE_INTERNAL_RESET,
            "",
            {}));
      }
    }
  }

  void App::pairActivate()
  {
    std::string ssid = fireAlarm::DataCache::instance().get(SSID);
    std::string mac = fireAlarm::DataCache::instance().get(MAC_ADDR);
    if (ssid.length() > 0 && mac.length() > 0)
    {

      if (ssid.length() < 6)
      {
        ssid.append("######");
      }

      std::string deviceIdBase64 = encode(
          reinterpret_cast<const uint8_t *>(ssid.c_str()),
          ssid.length());
      std::string endpoint_pair_activate = static_cast<std::string>(PREFIX_ENDPOINT) +
                                           "/device/pair?deviceId=" + deviceIdBase64;
      sock_->send(
          HTTPPacket(
              HTTPMethod::GET,
              endpoint_pair_activate,
              {{"UserAgent", "iotDevice"},
               {"Host",
                fireAlarm::DataCache::instance().get(CLOUD_ENDPOINT)},
               {"key", DEFAULT_TYPE_KEY},
               {"macAddress", mac}},
              {}));
    }
    else
    {
      Log::error(App_TAG, "ssid and mac not found");
    }
  }

  void App::getConfigServer()
  {
    auto externalId = fireAlarm::DataCache::instance().get(EXTERNAL_ID);
    auto externalKey = fireAlarm::DataCache::instance().get(EXTERNAL_KEY);
    if (externalId.length() > 0 && externalKey.length() > 0)
    {
      if (sock_)
      {
        auto isSent = sock_->send(
            HTTPPacket(
                HTTPMethod::GET,
                static_cast<std::string>(PREFIX_ENDPOINT) +
                    "/device/getConfig?externalId=" + externalId + "&externalKey=" + externalKey,
                {{"Host", fireAlarm::DataCache::instance().get(CLOUD_ENDPOINT)}},
                {}));
        if (!isSent)
        {
          Log::error(App_TAG, "call getConfig failed");
        }
      }
    }
    else
    {
      Log::error(App_TAG, "deviceId and token not found");
    }
  }

  void App::establishSocketHttp()
  {
    if (!sock_)
    {
      buff_ = std::make_shared<BufferContainer<Proto>>(
          *this, *this, *this, *this,
          std::make_unique<smooth::application::network::http::HTTPProtocol>(1024, 2048, *this));

#if IS_SSL_REST_API == 0
      sock_ = smooth::core::network::Socket<Proto>::create(
          buff_,
          std::chrono::milliseconds{90000},
          std::chrono::milliseconds{90000});
#else
      auto ca_cert = get_certs();

      tls_context = std::make_unique<MBedTLSContext>();
      tls_context->init_client(*ca_cert);
      sock_ = smooth::core::network::SecureSocket<Proto>::create(
          buff_,
          tls_context->create_context(),
          std::chrono::milliseconds{90000},
          std::chrono::milliseconds{90000});
#endif

      start_connect_server();
    }
  }

  void App::start_connect_server()
  {
    if (sock_)
    {
      auto cloud_endpoint = fireAlarm::DataCache::instance().get(CLOUD_ENDPOINT);
      auto cloud_port = stoi(fireAlarm::DataCache::instance().get(CLOUD_PORT));
      bool is_start_socket = sock_->start(
          std::make_shared<IPv4>(
              cloud_endpoint,
              cloud_port));
      // Log::error(App_TAG, "is_start_socket: {0}", is_start_socket);
      if (!is_start_socket)
      {
        Publisher<led::EventLed>::publish(
            led::EventLed("error"));
        // try_connect_server_++;
        // if (try_connect_server_ >= 10) {
        //   try_connect_server_ = 0;
        //   return;
        // }
        // start_connect_server();
        return;
      }
    }
  }
}