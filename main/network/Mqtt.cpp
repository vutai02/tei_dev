#include <chrono>
#include <smooth/core/logging/log.h>
#include <smooth/core/network/IPv4.h>
#include <smooth/core/ipc/Publisher.h>
#include <smooth/core/json/JsonFile.h>
#include <smooth/core/util/json_util.h>
#include <smooth/core/task_priorities.h>
#include "Mqtt.h"
#include "utils/DataCache.hpp"
#include "utils/CaCertPem.h"
#include "common/ConstantType.h"
#include "common/ConfigConstant.h"
#include "storage/StorageInfoDevice.h"

using namespace fireAlarm::storage;

using namespace std::chrono;
using namespace smooth::core;
using namespace fireAlarm::common;
using namespace smooth::core::ipc;
using namespace smooth::core::json;
using namespace smooth::core::network;
using namespace smooth::core::json_util;
using namespace smooth::application::network::mqtt;

// http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html

Mqtt::Mqtt(std::string id,
  smooth::core::Task& task,
  fireAlarm::CommandDispatcher& cmd)
  : task_(task),
    cmd_(cmd),
    incoming_mqtt_(MQTTQueue::create(10, task_, *this)),
    id_(std::move(id)),
    data_(DataDev2SerQueue::create(6, task_, *this))
{
}

Mqtt::~Mqtt()
{
  if(client_) {
    client_->disconnect();
  }
}

void Mqtt::start()
{
  if(!client_) {
    bool mqtt_enable = true;
    auto keep_alive = std::chrono::seconds(120);
    std::string broker = fireAlarm::DataCache::instance().get(MQTT_ENDPOINT);
    int port = stoi(fireAlarm::DataCache::instance().get(MQTT_PORT));

    std::string username = fireAlarm::DataCache::instance().get(USER);
    std::string password = fireAlarm::DataCache::instance().get(PASS);
    
    if (mqtt_enable) {
      if (broker.empty()) {
        Log::error(id_, "No broker specifified");
      }
      else {
        Log::info(id_, "Starting MQTT client, id {}", id_);
        Log::info(id_, "Starting MQTT broker, {}", broker);
        Log::info(id_, "Starting MQTT port, {}", port);
        client_ = std::make_unique<MqttClient>(
          id_,
          keep_alive,
          4 * 1024,
          APPLICATION_BASE_PRIO,
          incoming_mqtt_
        );

        if (fireAlarm::DataCache::instance().get(SCHEME) == "mqtts") {
          auto ca_cert = get_certs();
          client_->load_certificate(*ca_cert);
        }

        for(const auto& t : subscriptions_) {
          Log::info(id_, "subscribe channel {}", t);
          client_->subscribe(t, QoS::AT_LEAST_ONCE);
        }
        
        client_->set_authorization(username, password);
        client_->connect_to(std::make_shared<IPv4>(broker, port), true);
      }
    }
  }
}

void Mqtt::set_authorization(const std::string& user, const std::string& pass)
{
  if (client_) {
    client_->set_authorization(user, pass);
  }
}

bool Mqtt::connect_to(const std::string& broker, int port)
{
  if (broker.empty()) {
    return false;
  }
  if (!client_) {
    return false;
  }
  client_->connect_to(std::make_shared<IPv4>(broker, port), true);

  return true;
}

void Mqtt::subscription()
{
  if (client_) {
    for(const auto& t : subscriptions_) {
      Log::info(id_, "subscribe channel {}", t);
      client_->subscribe(t, QoS::AT_LEAST_ONCE);
    }
  }
}


void Mqtt::prepare_packet(nlohmann::json& v)
{
  auto time = duration_cast<seconds>(system_clock::now().time_since_epoch());
  v[TOKEN] = "";
  v[DATE_TIME] = std::to_string(time.count());
}

void Mqtt::add_subscription(const std::string& topic)
{
  if( std::find(
      std::begin(subscriptions_),
      std::end(subscriptions_),
      topic) 
    == std::end(subscriptions_)) {
    subscriptions_.emplace_back(topic);
  }
}

/**
 * payload:
 * {
 *   TYPE: "schedule",
 *   DATA: object,
 *   "dateTime": 1523455,
 *   "token": "356uihdfoa"
 * }
 * 
*/

void Mqtt::event(const smooth::application::network::mqtt::MQTTData &data)
{
  try {
    const auto& payload = 
      smooth::application::network::mqtt::MqttClient::get_payload(data);    
    nlohmann::json payloadJson = nlohmann::json::parse(payload);
    
    Log::info(id_, "Server to Device:::: {}", payloadJson.dump());

    if (payloadJson.contains(TYPE)
      && default_value(payloadJson, TYPE, "").length()
      && payloadJson[DATA].is_object()) {
      cmd_.process(default_value(payloadJson, TYPE, ""), payloadJson[DATA].dump(), default_value(payloadJson, ID, ""));
    }
  } catch(...) {
    Log::error(id_, "Parser json error");
  }
}

void Mqtt::event(const ObjectDataDev2Ser &value)
{
  if (client_) {
    nlohmann::json v{};
    v[TYPE] = value.get_type();
    if (value.get_id_package().length()) {
      v[ID] = value.get_id_package();
    }

    auto& data = v[DATA];
    data[ID] = fireAlarm::DataCache::instance().get(USER);
    data[SOURCE] = "Device";
    data[ACTION] = value.get_action();
    data[DATA] = value.get_json();
    std::string topicNotify = "channels/";
    topicNotify.append(fireAlarm::DataCache::instance().get("notify"));
    topicNotify.append("/messages");
    
    smooth::application::network::mqtt::QoS qos = AT_LEAST_ONCE;

    if (value.get_type() == TYPE_EXTERNAL_PING_2_SERVER) {
      qos = QoS::AT_MOST_ONCE;
    }

    send(topicNotify, v, qos);
  }
}

void Mqtt::send(const std::string& topic, nlohmann::json& v, smooth::application::network::mqtt::QoS qos)
{
  prepare_packet(v);
  if (client_->is_connected()) {
    Log::info(id_, "----> Send 2 server:::: {}", v.dump());
    client_->publish(topic, v.dump(), qos, false);
  }
  else {
    Log::error(id_, "Not connected to the server");
  }
}