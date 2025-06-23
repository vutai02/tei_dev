#pragma once
#include <smooth/core/Application.h>
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_event.h"
#include "esp_log.h"
#include "ethernet_init.h"
#include "sdkconfig.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include <smooth/core/io/Output.h>
#include "esp_mac.h"
#include "common/ObjectDataSendToMetro.h"
#include "common/ConstantType.h"
#include "common/ConfigConstant.h"
#include "storage/nvs/StorageNvsE.h"
#include <nlohmann/json.hpp>
#include <smooth/core/json/JsonFile.h>
#include <smooth/core/util/json_util.h>
#include <smooth/core/ipc/SubscribingTaskEventQueue.h>
#include <smooth/core/ipc/Publisher.h>
#include <smooth/core/timer/Timer.h>
#include <smooth/core/timer/TimerExpiredEvent.h>

namespace fireAlarm
{
    namespace network
    {
        class EthAdapter
        {
        public:
            EthAdapter()
            {
            }
            void start();
            void stop();
            void reconnect();
        };
    }
}