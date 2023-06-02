#include "Ping.h"

#include <string>
#include <smooth/core/ipc/Publisher.h>
#include "common/ObjectMetroPingServer.h"
#include "common/ConstantType.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include <ping/ping_sock.h>
#pragma GCC diagnostic pop

using namespace smooth::core::ipc;
using namespace smooth::core::logging;
using namespace gateway::common;

volatile bool is_ping = false;

namespace ping
{

  static void on_ping_success(esp_ping_handle_t hdl, void *args)
  {
    uint8_t ttl;
    uint16_t seqno;
    uint32_t elapsed_time, recv_len;
    ip_addr_t target_addr;
    esp_ping_get_profile(hdl, ESP_PING_PROF_SEQNO, &seqno, sizeof(seqno));
    esp_ping_get_profile(hdl, ESP_PING_PROF_TTL, &ttl, sizeof(ttl));
    esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));
    esp_ping_get_profile(hdl, ESP_PING_PROF_SIZE, &recv_len, sizeof(recv_len));
    esp_ping_get_profile(hdl, ESP_PING_PROF_TIMEGAP, &elapsed_time, sizeof(elapsed_time));

    char data[70] = {0};
    int len = sprintf(data, "%d bytes from %s icmp_seq=%d ttl=%d time=%d ms",
          recv_len, inet_ntoa(target_addr.u_addr.ip4), seqno, ttl, elapsed_time);
    if (len) {
      Publisher<ObjectMetroPingServer>::publish(
        ObjectMetroPingServer(
          data
        )
      );
    }
  }

  static void on_ping_timeout(esp_ping_handle_t hdl, void *args)
  {
    uint16_t seqno;
    ip_addr_t target_addr;
    esp_ping_get_profile(hdl, ESP_PING_PROF_SEQNO, &seqno, sizeof(seqno));
    esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));

    char data[50] = {0};
    int len = sprintf(data, "From %s icmp_seq=%d timeout", inet_ntoa(target_addr.u_addr.ip4), seqno);
    if (len) {
      Publisher<ObjectMetroPingServer>::publish(
        ObjectMetroPingServer(
          data
        )
      );
    }
  }

  static void on_ping_end(esp_ping_handle_t hdl, void *args)
  {
    ip_addr_t target_addr;
    uint32_t transmitted;
    uint32_t received;
    uint32_t total_time_ms;
    esp_ping_get_profile(hdl, ESP_PING_PROF_REQUEST, &transmitted, sizeof(transmitted));
    esp_ping_get_profile(hdl, ESP_PING_PROF_REPLY, &received, sizeof(received));
    esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));
    esp_ping_get_profile(hdl, ESP_PING_PROF_DURATION, &total_time_ms, sizeof(total_time_ms));
    uint32_t loss = (uint32_t)((1 - ((float)received) / transmitted) * 100);
    char data[150] = {0};

    // if (IP_IS_V4(&target_addr)) {
    //   sprintf(data, "--- %s ping statistics ---\n", inet_ntoa(*ip_2_ip4(&target_addr)));
    // } else {
    //   sprintf(data, "--- %s ping statistics ---\n", inet6_ntoa(*ip_2_ip6(&target_addr)));
    // }

    int len = sprintf(data, "%d packets transmitted, %d received, %d%% packet loss, time %dms",
          transmitted, received, loss, total_time_ms);
    if (len) {
      Publisher<ObjectMetroPingServer>::publish(
        ObjectMetroPingServer(
          data
        )
      );
    }

    // delete the ping sessions, so that we clean up all resources and can create a new ping session
    // we don't have to call delete function in the callback, instead we can call delete function from other tasks
    esp_ping_delete_session(hdl);
    is_ping = false;
  }

  Ping::Ping(smooth::core::Task& task)
    : task(task),
    ping(ObjectPingServerQueue::create(3, task, *this))
  {
  }

  void Ping::event(const ObjectPingServer& ev)
  {
    if (!is_ping) {
      this->start(ev.get_add(), ev.get_len());
      is_ping = true;
    }
  }

  void Ping::start(const std::string& add, int len)
  {
    ip_addr_t target_addr;
    struct addrinfo hint;
    struct addrinfo *res = NULL;
    memset(&hint, 0, sizeof(hint));
    memset(&target_addr, 0, sizeof(target_addr));
    int err = getaddrinfo(add.c_str(), NULL, &hint, &res);
    if(err != 0 || res == NULL) {
      // ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p", err, res);
      return;
    }
    struct in_addr addr4 = ((struct sockaddr_in *) (res->ai_addr))->sin_addr;
    inet_addr_to_ip4addr(ip_2_ip4(&target_addr), &addr4);
    freeaddrinfo(res);

    esp_ping_config_t ping_config = ESP_PING_DEFAULT_CONFIG();
    ping_config.target_addr = target_addr;          // target IP address
    ping_config.count = len;    // ping in infinite mode, esp_ping_stop can stop it

    /* set callback functions */
    esp_ping_callbacks_t cbs = {
      .cb_args = NULL,
      .on_ping_success = on_ping_success,
      .on_ping_timeout = on_ping_timeout,
      .on_ping_end = on_ping_end
    };
    esp_ping_handle_t ping;
    esp_ping_new_session(&ping_config, &cbs, &ping);
    esp_ping_start(ping);
  }

  void Ping::stop()
  {
    // esp_ping_stop(ping);
    // esp_ping_delete_session(ping);
  }
}