#pragma once

namespace iotTouch::common
{
  constexpr const char* CONFIGURATION = "cfg";
  constexpr const char* MQTT = "mqtt";
  
  constexpr const char* DEVICE = "dev";
  constexpr const char* ID = "id";
  constexpr const char* TOKEN = "tok";

  constexpr const char* ENABLE = "enable";
  constexpr const char* SERVER = "server";
  constexpr const char* PORT = "port";
  constexpr const char* CLIENT_ID = "client_id";
  constexpr const char* USER = "user";
  constexpr const char* PASS = "pass";
  constexpr const char* RECONNECT_TIMEOUT_MIN = "timeout_min";
  constexpr const char* RECONNECT_TIMEOUT_MAX = "timeout_max";
  constexpr const char* SSL_CERT = "cert";
  constexpr const char* SSL_KEY = "key";
  constexpr const char* SSL_CA_CERT = "ca";
  constexpr const char* CLEAN_SESSION = "clean_session";
  constexpr const char* KEEP_ALIVE = "keep_alive";
  constexpr const char* MAX_QOS = "max_qos";

  constexpr const char* WIFI = "wifi";
  constexpr const char* AP = "ap";
  constexpr const char* STA = "sta";    
  constexpr const char* SSID = "ssid";
  constexpr const char* KEY = "key";
  constexpr const char* IP = "ip";

  constexpr const char* EXTERNAL_ID = "externalId";
  constexpr const char* EXTERNAL_KEY = "externalKey";

  constexpr const char* DATA = "data";
  constexpr const char* VALUE = "value";

  constexpr const char* DEVICE_ID = "device_id";
  constexpr const char* SERVER_ID = "id_ser";

  constexpr const char* MAC_ADDR = "mac";

  constexpr const char* CLOUD_SKY_TECH = "cloud_sky";

  constexpr const char* ENABLED = "enabled";
  constexpr const char* INPUT = "input";
  constexpr const char* NAME = "name";
  constexpr const char* MIN = "min";
  constexpr const char* MAX = "max";
  constexpr const char* OUTPUT = "output";
  constexpr const char* SECTION = "section";
  constexpr const char* SENSORS = "sensors";
  constexpr const char* STATE = "status";

  constexpr const char* TOPIC = "topic";
  constexpr const char* ACTIVATE = "activate";

  constexpr const char* SCHEDULES = "sch";
  constexpr const char* COUNTDOWN = "cnt";

  constexpr const char* INDEX = "index";
  constexpr const char* CRON = "cron";
  constexpr const char* TYPE = "type";
  constexpr const char* ACTION = "action";
  constexpr const char* NOTIFICATION = "notify";
  constexpr const char* TIMER = "timer";
  constexpr const char* REPEAT = "repeat";

  constexpr const char* WIFI_MODE = "smartconfig";
  constexpr const char* DATE_TIME = "dateTime";
  constexpr const char* SOURCE = "source";
  
  constexpr const char* VERSION = "version";
  constexpr const char* MODEL = "model";
  constexpr const char* ENVIRONMENT = "environment";
  constexpr const char* PRODUCTION_DATE = "productionDate";
  constexpr const char* CUSTOMER = "customer";
  constexpr const char* MANUFACTURE = "manufacture";
  constexpr const char* FACTORY = "factory";



  // constexpr const char* DEFAULT_ENDPOINT_CLOUD = "149.28.138.168";
  constexpr const char* DEFAULT_ENDPOINT_CLOUD = "product.skytechnology.vn";
  constexpr const int DEFAULT_PORT_CLOUD = 7819;
  // constexpr const int DEFAULT_PORT_CLOUD = 7876;
  constexpr const int DEFAULT_PORT_MQTT_CLOUD = 1883;
  
#define IS_SSL_REST_API         0
#define TOUCH_TYPE              3

  constexpr const char* DEFAULT_TYPE_KEY = "touch_3";

  constexpr const char* MQTT_PORT = "mqtt_p";
  constexpr const char* MQTT_ENDPOINT = "mqtt_s";
  constexpr const char* CLOUD_PORT = "cloud_p";
  constexpr const char* CLOUD_ENDPOINT = "cloud_s";
  constexpr const char* SCHEME = "scheme";
  
  constexpr const char* ENV = "production";
  constexpr const char* PREFIX_ENDPOINT = "/api/v1.0";

  enum WifiMode {
    ap_mode = 0,
    softap_mode = 1,
    smartconfig_mode = 2,
    provisioning_mode = 3,
    reset_mode = 4,
  };
}