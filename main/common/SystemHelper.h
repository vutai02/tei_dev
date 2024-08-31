#pragma once 

#include "esp_sleep.h"
#include "esp_system.h"
#include "esp32s3/rom/rtc.h"

namespace iotTouch::system
{
  class SystemHelper
  {
    public:
      SystemHelper()
      {
      }

      void restartSystem()
      {
        esp_sleep_enable_timer_wakeup(2000);
        esp_deep_sleep_start();
      }

      void restartChip()
      {
        esp_restart();
      }
    private:
  };
}