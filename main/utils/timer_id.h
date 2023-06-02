#pragma once

// Note for anyone reading this: Smooth timer IDs only need to be unique when more than one timer
// is sending to the same listener, but we're keeping them all uniqueue just for good measure.
constexpr const int ALARM_TICK = 1001;
constexpr const int KEYPAD_ENTRY_TIMEOUT = 1002;
constexpr const int PLAYER_ID = 1003;
constexpr const int PUBLISH_OUTPUTS = 1004;
constexpr const int ACTIVATE_TICK = 1005;
constexpr const int REFACTORY_TICK = 1006;
constexpr const int SCHEDULES_TICK = 1007;
constexpr const int SYNC_RELAY_TICK = 1008;
constexpr const int STROBE_LIGHT_TICK = 1009;
constexpr const int TRIGER_RELAY_TICK = 1011;
constexpr const int SYNC_MQTT_TICK = 1010;