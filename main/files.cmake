
set(SOURCES main.cpp
  App.cpp
  App.h
  utils/DeviceId.cpp
  utils/DeviceId.h
    # io/output/outputAlarm.cpp
  # io/output/outputAlarm.h
  # network/MulticastDNS.h
  

  io/Button/button.cpp
  io/Button/button.h
  io/input/input.cpp
  io/input/input.h
  io/IoController.h
  io/IoController.cpp

  # RS485_master/core_master/master.cpp
  # RS485_master/core_master/master.h
  # RS485_master/define.h
  # RS485_master/rs485.cpp
  # RS485_master/rs485.h

  # RS485_slave/slave/slave.cpp
  # RS485_slave/slave/slave.h
  # RS485_slave/RS485.cpp
  # RS485_slave/RS485.h
  # RS485_slave/define.h

  rs485/core_master/Core.cpp
  rs485/core_master/Core.h
  rs485/define_res.h
  rs485/rs485.cpp
  rs485/rs485.h

  
  network/Ethernet.cpp
  network/Ethernet.h
  network/WifiAdapter.h
  network/WifiAdapter.cpp
  network/Sntp.h
  network/Sntp.cpp
  network/Mqtt.h
  network/Mqtt.cpp
  common/ConfigConstant.h
  utils/CommandDispatcher.h
  utils/CommandDispatcher.cpp
  common/digital/DigitalInputValue.h
  common/digital/DigitalOutputValue.h
  common/digital/SensitivityTouchValue.h
  common/digital/DigitalStatusValue.h
 
  schedules/CronAlarms.h
  schedules/CronAlarms.cpp
  common/SyncTimerSer.h
  schedules/ccronexpr/ccronexpr.c
  schedules/ccronexpr/ccronexpr.h
  common/digital/DigitalCountDown.h
  schedules/SchedulesController.h
  schedules/SchedulesController.cpp
  common/ObjectSetDigitalState2Storage.h
  common/ObjectSetCountDown.h
  common/ObjectSetTimer.h
  storage/Storage.cpp
  storage/Storage.h
  storage/StorageWifi.h
  storage/StorageWifi.cpp
  storage/StorageSchedule.h
  storage/StorageSchedule.cpp
  storage/StorageIoTouch.h
  storage/StorageIoTouch.cpp
  storage/StorageInfoDevice.h
  storage/StorageInfoDevice.cpp
  storage/nvs/NvsEsp32.h
  storage/nvs/NvsEsp32.cpp
  storage/nvs/StorageNvsE.h
  storage/nvs/StorageNvsE.cpp
  utils/timer_id.h
  utils/Utils.h
  utils/Utils.cpp
  utils/CaCertPem.h
  utils/CaCertPem.cpp

  common/ConstantType.h
  common/SystemHelper.h
  ota/Ota.h
  ota/Ota.cpp
  metroController/MetroController.h
  metroController/MetroController.cpp
  led/LedController.h
  led/LedController.cpp
)