#pragma once

namespace fireAlarm::common
{
  constexpr const char* TYPE_EXTERNAL_CONTROL_TOUCH    = "DA01";
  constexpr const char* TYPE_EXTERNAL_SET_COUNTDOWN    = "DA03";
  constexpr const char* TYPE_EXTERNAL_SET_SCHEDULE     = "DA02";
  constexpr const char* TYPE_EXTERNAL_PING_2_SERVER    = "CP01";
  constexpr const char* TYPE_EXTERNAL_STATUS_2_SERVER  = "DA04";
  constexpr const char* TYPE_EXTERNAL_ACK_2_SERVER     = "DA05";
  constexpr const char* TYPE_EXTERNAL_OTA              = "DA06";
  constexpr const char* TYPE_EXTERNAL_VERSION          = "DA07";
  constexpr const char* TYPE_EXTERNAL_PING_ICMP        = "ICMP";
  constexpr const char* TYPE_EXTERNAL_GET_SCHEDULES    = "DA08";
  constexpr const char* TYPE_EXTERNAL_SET_SLEEP_LED_RED = "DA09";
  constexpr const char* TYPE_EXTERNAL_SEND_TOUCH_SETTING = "DA07";
  
  constexpr const char* TYPE_INTERNAL_SET_SCHEDULE     = "SCHEDULE";
  constexpr const char* TYPE_INTERNAL_RESET            = "RESET";
  constexpr const char* TYPE_INTERNALL_TOUCH_SETTING   = "DEV_INFO";


  constexpr const char *TYPE_INTERNAL_ETH = "ETH";

  constexpr const char *RECONNECT_ETH = "re_eth";
  constexpr const char *STOP_ETH = "stop_eth";


  constexpr const char *TYPE_EXTERNAL_TEM_HU = "tem_hu";

  // #define TOUCH_BUTTON_MAX_CHANGE_RATE_1   0.0249
  // #define TOUCH_BUTTON_MAX_CHANGE_RATE_2   0.0225
  // #define TOUCH_BUTTON_MAX_CHANGE_RATE_3   0.02684
  // #define TOUCH_BUTTON_MAX_CHANGE_RATE_4   0.0276

  // #define TOUCH_BUTTON_MAX_CHANGE_RATE_HOT   0.0187

  // #define TOUCH_BUTTON_MAX_CHANGE_RATE_1   0.0175   // TOUCH_PAD_NUM2
  // #define TOUCH_BUTTON_MAX_CHANGE_RATE_2   0.0175   // TOUCH_PAD_NUM3
  // #define TOUCH_BUTTON_MAX_CHANGE_RATE_3   0.0175   // TOUCH_PAD_NUM4
  // #define TOUCH_BUTTON_MAX_CHANGE_RATE_4   0.0195   // TOUCH_PAD_NUM0

  #define TOUCH_BUTTON_MAX_CHANGE_RATE_1   0.0185   // TOUCH_PAD_NUM2
  #define TOUCH_BUTTON_MAX_CHANGE_RATE_2   0.0185   // TOUCH_PAD_NUM3
  #define TOUCH_BUTTON_MAX_CHANGE_RATE_3   0.0185   // TOUCH_PAD_NUM4
  #define TOUCH_BUTTON_MAX_CHANGE_RATE_4   0.0185   // TOUCH_PAD_NUM0

  #define TOUCH_BUTTON_MAX_CHANGE_RATE_HOT 1.1

  const int TOUCH_LOGGING_LEVEL           = 4;
  const int TOUCH_SCHEDULES_LOGGING_LEVEL = 4;
  const int TOUCH_OTA_LOGGING_LEVEL       = 4;
  const int TOUCH_IO_LOGGING_LEVEL        = 4;
  const int TOUCH_METRO_LOGGING_LEVEL     = 4;
}