#pragma once

#include <string>
#include <chrono>
#include <vector>
#include <nlohmann/json.hpp>
#include <smooth/core/Task.h>
#include <smooth/core/ipc/TaskEventQueue.h>
#include <smooth/core/ipc/SubscribingTaskEventQueue.h>
#include <smooth/application/network/mqtt/MqttClient.h>
#include "utils/CommandDispatcher.h"
#include "common/ObjectDataDev2Ser.h"

class Mqtt : 
  public smooth::core::ipc::IEventListener<smooth::application::network::mqtt::MQTTData>,
  public smooth::core::ipc::IEventListener<ObjectDataDev2Ser>
{
  public:
    Mqtt(std::string id, smooth::core::Task& task, iotTouch::CommandDispatcher& cmd);
    ~Mqtt() override;
    
    void event(const smooth::application::network::mqtt::MQTTData& data) override;

    void event(const ObjectDataDev2Ser &value) override;

    void start();

    void add_subscription(const std::string& topic);
    [[nodiscard]] bool is_connected() const { return client_ && client_->is_connected(); }

    [[nodiscard]] int get_cnt_reconnect_broker() const { 
      if (client_) {
        return client_->get_cnt_reconnect();
      }
      return 0;
    }

    void reset_cnt_reconnect_broker() { 
      if (client_) {
        client_->reset_cnt_reconnect();
      }
    }

    void disconnect_broker() { 
      if (client_) {
        client_->disconnect();
      }
    }

    void set_authorization(const std::string& user, const std::string& pass);
    bool connect_to(const std::string& broker, int port);
    void subscription();

  private:
    void start_mqtt();
    void prepare_packet(nlohmann::json& v);
    void send(
      const std::string& topic,
      nlohmann::json& v,
      smooth::application::network::mqtt::QoS qos = smooth::application::network::mqtt::QoS::AT_LEAST_ONCE);

    smooth::core::Task& task_;
    iotTouch::CommandDispatcher& cmd_;
    
    using MQTTQueue = smooth::core::ipc::TaskEventQueue<smooth::application::network::mqtt::MQTTData>;

    std::shared_ptr<MQTTQueue> incoming_mqtt_;

    std::unique_ptr<smooth::application::network::mqtt::MqttClient> client_{};
    std::vector<std::string> subscriptions_{};
    std::string id_;

    using DataDev2SerQueue = smooth::core::ipc::SubscribingTaskEventQueue<ObjectDataDev2Ser>;
    std::shared_ptr<DataDev2SerQueue> data_;
};