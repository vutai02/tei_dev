#pragma once

typedef struct{
  uint8_t state;
  bool is_push;
  uint8_t tp;
  uint8_t id;
  bool value;
  bool is_ack;
  unsigned long timer;
}__attribute__((__packed__)) iot_tp_dev_t;

typedef struct{
  uint8_t state;
  bool is_local;
  bool value;
  uint8_t id;
}__attribute__((__packed__)) cache_io_relay_t;
