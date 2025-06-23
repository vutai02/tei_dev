#pragma once

namespace fireAlarm::common
{
  constexpr const char *CONFIGURATION = "cfg";
  constexpr const char *MQTT = "mqtt";
  constexpr const char *UPDATE = "Update";

  constexpr const char *DEVICE = "dev";
  constexpr const char *ID = "id";
  constexpr const char *TOKEN = "tok";

  constexpr const char *ENABLE = "enable";
  constexpr const char *SERVER = "server";
  constexpr const char *PORT = "port";
  constexpr const char *CLIENT_ID = "client_id";
  constexpr const char *USER = "user";
  constexpr const char *PASS = "pass";
  constexpr const char *RECONNECT_TIMEOUT_MIN = "timeout_min";
  constexpr const char *RECONNECT_TIMEOUT_MAX = "timeout_max";
  constexpr const char *SSL_CERT = "cert";
  constexpr const char *SSL_KEY = "key";
  constexpr const char *SSL_CA_CERT = "ca";
  constexpr const char *CLEAN_SESSION = "clean_session";
  constexpr const char *KEEP_ALIVE = "keep_alive";
  constexpr const char *MAX_QOS = "max_qos";

  constexpr const char *WIFI = "wifi";
  constexpr const char *AP = "ap";
  constexpr const char *STA = "sta";
  constexpr const char *SSID = "ssid";
  constexpr const char *KEY = "key";
  constexpr const char *IP = "ip";
  constexpr const char *RSSI = "rssi";

  constexpr const char *EXTERNAL_ID = "externalId";
  constexpr const char *EXTERNAL_KEY = "externalKey";

  constexpr const char *DATA = "data";
  constexpr const char *VALUE = "value";
  constexpr const char *CONFIG = "configuration";
  constexpr const char *VARIABLE = "variables";
  constexpr const char *DEVICE_ID = "device_id";
  constexpr const char *SERVER_ID = "id_ser";
  constexpr const char *STATUS = "status";
  constexpr const char *MAC_ADDR = "mac";

  constexpr const char *CLOUD_SKY_TECH = "cloud_sky";

  constexpr const char *ENABLED = "enabled";
  constexpr const char *INPUT = "input";
  constexpr const char *NAME = "name";
  constexpr const char *MIN = "min";
  constexpr const char *MAX = "max";
  constexpr const char *OUTPUT = "output";
  constexpr const char *SECTION = "section";
  constexpr const char *SENSORS = "sensors";
  constexpr const char *STATE = "status";

  constexpr const char *TOPIC = "topic";
  constexpr const char *ACTIVATE = "activate";

  constexpr const char *SCHEDULES = "sch";
  constexpr const char *COUNTDOWN = "cnt";

  constexpr const char *INDEX = "index";
  constexpr const char *CRON = "cron";
  constexpr const char *TYPE = "type";
  constexpr const char *ACTION = "action";
  constexpr const char *NOTIFICATION = "notify";
  constexpr const char *TIMER = "timer";
  constexpr const char *REPEAT = "repeat";

  constexpr const char *WIFI_MODE = "smartconfig";
  constexpr const char *DATE_TIME = "dateTime";
  constexpr const char *SOURCE = "source";

  constexpr const char *VERSION = "version";
  constexpr const char *MODEL = "model";
  constexpr const char *ENVIRONMENT = "environment";
  constexpr const char *PRODUCTION_DATE = "productionDate";
  constexpr const char *CUSTOMER = "customer";
  constexpr const char *MANUFACTURE = "manufacture";
  constexpr const char *FACTORY = "factory";

  // constexpr const char *DEFAULT_ENDPOINT_CLOUD = "dev.skytechnology.vn";
  constexpr const char *DEFAULT_ENDPOINT_CLOUD = "product.skytechnology.vn";
  constexpr const int DEFAULT_PORT_CLOUD = 7819;
  // constexpr const int DEFAULT_PORT_CLOUD = 7879;
  constexpr const int DEFAULT_PORT_MQTT_CLOUD = 1883;

  constexpr const char *DEFAULT_TYPE_KEY = "fire_alarm"; // ir_gateway fire_alarm
  constexpr const char *TIMER_TRIGGER = "timeTrigger";
  constexpr const char *TIMER_TRIGGER_SE = "timeTriggerInput"; // willbe erase in next version
  constexpr const char *TIMER_OUTPUT = "timeOutput";
  constexpr const char *TIMER_FEEDBACK = "timeFeedback";
  constexpr const char *LEVEL_INPUT = "level_input";
  constexpr const char *MQTT_PORT = "mqtt_p";
  constexpr const char *MQTT_ENDPOINT = "mqtt_s";
  constexpr const char *CLOUD_PORT = "cloud_p";
  constexpr const char *CLOUD_ENDPOINT = "cloud_s";
  constexpr const char *SCHEME = "scheme";

  constexpr const char *ENV = "develop";
  constexpr const char *PREFIX_ENDPOINT = "/api/v1";

  constexpr const char *TEMP = "temperature";
  constexpr const char *HUMI = "humidity";

  constexpr const char *DELETE = "Delete";
  constexpr const char *INSERT = "Insert";

  constexpr const char *TRIGGER = "trigger";
  constexpr const char *NOT_TRIGGER = "not_trigger";

  constexpr const char *ENABLE_ETH = "enable_eth";
  constexpr const char *DISABLE_ETH = "disable_eth";
  constexpr const char *SETUP_ETH = "setup_eth";
  constexpr const char *SETWIFI = "SetWifi";

  constexpr const char *PRE_SSID = "pre_ssid";
  constexpr const char *PRE_KEY = "pre_key";

  enum WifiMode
  {
    ap_mode = 0,
    softap_mode = 1,
    smartconfig_mode = 2,
    provisioning_mode = 3,
    reset_mode = 4,
    change_ssid_pass = 5,
  };

}

#define TOUCH_TYPE 1

// #define IO_OUTPUT_ALARM_4 GPIO_NUM_21
// #define IO_OUTPUT_ALARM_3 GPIO_NUM_22 // 19
// #define IO_OUTPUT_ALARM_2 GPIO_NUM_17 // 23
// #define IO_OUTPUT_ALARM_1 GPIO_NUM_16 // 2

#define IO_CONFIG_WIFI GPIO_NUM_4

#define IO_OUTPUT_ALARM_1 GPIO_NUM_38
#define IO_OUTPUT_ALARM_2 GPIO_NUM_39 
#define IO_OUTPUT_ALARM_3 GPIO_NUM_40 


#define IO_INPUT_ALARM_1 GPIO_NUM_41
#define IO_INPUT_ALARM_2 GPIO_NUM_42

// #define IO_LED GPIO_NUM_4 
#define IO_LED1 GPIO_NUM_2
#define IO_LED2 GPIO_NUM_3

// #define IO_INPUT15 GPIO_NUM_22
// #define IO_INPUT14 GPIO_NUM_15 // 13
#define IO_INPUT13 GPIO_NUM_13 // 12
#define IO_INPUT12 GPIO_NUM_14
#define IO_INPUT11 GPIO_NUM_27
// #define IO_INPUT10 GPIO_NUM_35
// #define IO_INPUT9 GPIO_NUM_32
// #define IO_INPUT8 GPIO_NUM_33

// #define IO_INPUT7 GPIO_NUM_25
// #define IO_INPUT6 GPIO_NUM_26
#define IO_INPUT5 GPIO_NUM_36
#define IO_INPUT4 GPIO_NUM_37
#define IO_INPUT3 GPIO_NUM_38
#define IO_INPUT2 GPIO_NUM_39
#define IO_INPUT1 GPIO_NUM_34

// #define IO_CONFIG_WIFI GPIO_NUM_2 

// #define SCL_PIN GPIO_NUM_4
// #define SDA_PIN GPIO_NUM_15

#define IS_SSL_REST_API 0