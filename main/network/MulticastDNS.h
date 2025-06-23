#pragma once

#include <string>
#include "mdns.h"

namespace fireAlarm::network
{
  #define MDNS_HOSTNAME "skytech"
  #define MDNS_INSTANCE "fireAlarm"
  #define MDNS_SERVICE_NAME  "_http"
  #define MDNS_SERVICE_PROTO "_tcp"
  #define MDNS_SERVICE_PORT   80

  void initializeMdns() {
    //initialize mDNS
    mdns_init();
    //set mDNS hostname (required if you want to advertise services)
    mdns_hostname_set(MDNS_HOSTNAME);
    //set default mDNS instance name
    mdns_instance_name_set(MDNS_INSTANCE);
    //structure with TXT records
    mdns_txt_item_t serviceTxtData[3] = {
      {"fireAlarm","esp32"}
    };
    //initialize service
    mdns_service_add("touch-skytech", "_http", "_tcp", 80, serviceTxtData, 1);
  }
}