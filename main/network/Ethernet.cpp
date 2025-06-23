#include "Ethernet.h"
#include <smooth/core/logging/log.h>
#include <smooth/core/util/string_util.h>
#include <smooth/core/json/JsonFile.h>
#include <smooth/core/util/json_util.h>
#include "common/ConfigConstant.h"
using namespace fireAlarm::storage;
using namespace fireAlarm::common;
using namespace smooth::core::json;
using namespace smooth::core::logging;
using namespace smooth::core::ipc;
using namespace std::chrono;
namespace fireAlarm
{
    namespace network
    {
        static const char *TAG = "eth_";
        
        bool is_use_ethernet = false;
        esp_eth_handle_t eth_handle_ = NULL;
        eth_dev_info_t info;
        esp_eth_handle_t eth_handle_spi = NULL;
        esp_netif_t *eth_netif_spi = NULL;

        // static uint8_t eth_cnt_g = 0;
        // static eth_device eth_instance_g[CONFIG_ETHERNET_INTERNAL_SUPPORT + CONFIG_ETHERNET_SPI_NUMBER];

        static void EthernetHardwareEventHandler(void *arg, esp_event_base_t event_base,
                                                 int32_t event_id, void *event_data)
        {
            uint8_t pin1 = 0, pin2 = 0;
            uint8_t mac_addr[6] = {0};
            /* we can get the ethernet driver handle from event data */
            esp_eth_handle_t eth_handle = *(esp_eth_handle_t *)event_data;
            eth_dev_info_t dev_info = ethernet_init_get_dev_info(&eth_handle);

            
            if (dev_info.type == ETH_DEV_TYPE_INTERNAL_ETH) {
                pin1 = dev_info.pin.eth_internal_mdc;
                pin2 = dev_info.pin.eth_internal_mdio;
            } else if (dev_info.type == ETH_DEV_TYPE_SPI) {
                pin1 = dev_info.pin.eth_spi_cs;
                pin2 = dev_info.pin.eth_spi_int;
            }

            switch (event_id)
            {
            case ETHERNET_EVENT_CONNECTED:
                esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr);
                Log::info(TAG, "Ethernet HW Addr %02x:%02x:%02x:%02x:%02x:%02x",
                         mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
                Log::info(TAG, "Ethernet(%s[%d,%d]) Link Up", dev_info.name, pin1, pin2);
                Publisher<ObjectDataSendToMetro>::publish(
                    ObjectDataSendToMetro(
                        TYPE_INTERNAL_ETH,
                        SETUP_ETH,
                        {}));
                break;
            case ETHERNET_EVENT_DISCONNECTED:
                Log::info(TAG,"Ethernet(%s[%d,%d]) Link Down", dev_info.name, pin1, pin2);
                // publish topic for disconnect eth
                Publisher<ObjectDataSendToMetro>::publish(
                    ObjectDataSendToMetro(
                        TYPE_INTERNAL_ETH,
                        DISABLE_ETH,
                        {}));
                break;
            case ETHERNET_EVENT_START:
                Log::info(TAG,"Ethernet(%s[%d,%d]) started", dev_info.name, pin1, pin2);
                break;
            case ETHERNET_EVENT_STOP:
                Log::info(TAG,"Ethernet(%s[%d,%d]) Link stopped", dev_info.name, pin1, pin2);
                break;
            default:
                break;
            }
        }
        /** Event handler for IP_EVENT_ETH_GOT_IP */
        static void GetIpEventHandler(void *arg, esp_event_base_t event_base,
                                      int32_t event_id, void *event_data)
        {
            ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
            const esp_netif_ip_info_t *ip_info = &event->ip_info;

            Log::info(TAG, "Ethernet Got IP Address");
            Log::info(TAG, "~~~~~~~~~~~");
            ESP_LOGI(TAG, "ETHIP:" IPSTR, IP2STR(&ip_info->ip));
            ESP_LOGI(TAG, "ETHMASK:" IPSTR, IP2STR(&ip_info->netmask));
            ESP_LOGI(TAG, "ETHGW:" IPSTR, IP2STR(&ip_info->gw));
            Log::info(TAG, "~~~~~~~~~~~");
            Publisher<ObjectDataSendToMetro>::publish(
                ObjectDataSendToMetro(
                    TYPE_INTERNAL_ETH,
                    ENABLE_ETH,
                    {}));
            // publish topic use eth
        }
        void EthAdapter::start()
        {
            Log::info(TAG, "start ethenet-->");
            // Initialize TCP/IP network interface (should be called only once in application)
            ESP_ERROR_CHECK(esp_netif_init());
            // esp_event_loop_delete_default
            ESP_ERROR_CHECK(esp_event_loop_delete_default());
            // Create default event loop that running in background
            ESP_ERROR_CHECK(esp_event_loop_create_default());

            //  Create instance(s) of esp-netif for SPI Ethernet(s)


            esp_netif_inherent_config_t esp_netif_config = ESP_NETIF_INHERENT_DEFAULT_ETH();

            esp_netif_config_t cfg_spi{
                .base = &esp_netif_config,
                .stack = ESP_NETIF_NETSTACK_DEFAULT_ETH};

            char if_key_str[10];
            char if_desc_str[10];
            char num_str[3];
            itoa(0, num_str, 10);

            strcat(strcpy(if_key_str, "ETH_SPI_"), num_str);
            strcat(strcpy(if_desc_str, "eth"), num_str);
            esp_netif_config.if_key = if_key_str;
            esp_netif_config.if_desc = if_desc_str;
            esp_netif_config.route_prio = 30;

            eth_netif_spi = esp_netif_new(&cfg_spi);

            // Init MAC and PHY configs to default
            eth_mac_config_t mac_config_spi = ETH_MAC_DEFAULT_CONFIG();

            eth_phy_config_t phy_config_spi = ETH_PHY_DEFAULT_CONFIG();
            phy_config_spi.phy_addr = 1;
            phy_config_spi.autonego_timeout_ms = 0;
            phy_config_spi.reset_gpio_num = 21;

            spi_bus_config_t buscfg = {
                .mosi_io_num = GPIO_NUM_11,
                .miso_io_num = GPIO_NUM_13,
                .sclk_io_num = GPIO_NUM_12,
                .quadwp_io_num = -1,
                .quadhd_io_num = -1,
                .max_transfer_sz = 32};
            ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));
            // Configure SPI interface and Ethernet driver for specific SPI module

            spi_device_interface_config_t spi_devcfg = {
                .command_bits = 16, // Actually it's the address phase in W5500 SPI frame
                .address_bits = 8,  // Actually it's the control phase in W5500 SPI frame
                .mode = 0,
                .clock_speed_hz = 10 * 1000 * 1000,
                .spics_io_num = GPIO_NUM_10,
                .queue_size = 20};

            // Set remaining GPIO numbers and configuration used by the SPI module
             phy_config_spi.phy_addr = 1;
             Log::info(TAG, "start ethenet-->");
            eth_w5500_config_t w5500_config = ETH_W5500_DEFAULT_CONFIG(SPI2_HOST, &spi_devcfg);
            w5500_config.int_gpio_num = 14; // todo -> devkit ma INT4 a LEO na innym!
            Log::info(TAG, " ethenet 1-->");
            esp_eth_mac_t *mac_spi = esp_eth_mac_new_w5500(&w5500_config, &mac_config_spi);
            Log::info(TAG, " ethenet 2-->");
            esp_eth_phy_t *phy_spi = esp_eth_phy_new_w5500(&phy_config_spi);
            Log::info(TAG, " ethenet 3-->");
            esp_eth_config_t eth_config_spi = ETH_DEFAULT_CONFIG(mac_spi, phy_spi);
            Log::info(TAG, " ethenet 4-->");
            ESP_ERROR_CHECK(esp_eth_driver_install(&eth_config_spi, &eth_handle_spi));
            Log::info(TAG, " ethenet esp_eth_driver_install5-->");
            uint8_t mac_addr[6];
            esp_read_mac(mac_addr, ESP_MAC_ETH);
            Log::info(TAG, " ethenet 6-->");
            ESP_ERROR_CHECK(esp_eth_ioctl(eth_handle_spi, ETH_CMD_S_MAC_ADDR, mac_addr));
            Log::info(TAG, " ethenet 7-->");
            // Register user defined event handers
            ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &EthernetHardwareEventHandler, NULL));
            Log::info(TAG, " ethenet 8-->");
            ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &GetIpEventHandler, NULL));
            Log::info(TAG, " ethenet 9-->");
            // attach Ethernet driver to TCP/IP stack
            ESP_ERROR_CHECK(esp_netif_attach(eth_netif_spi, esp_eth_new_netif_glue(eth_handle_spi)));
            /* start Ethernet driver state machine */
            Log::info(TAG, "start ethenet-->");
            ESP_ERROR_CHECK(esp_eth_start(eth_handle_spi));

            Log::info(TAG, "start esp_eth_start-->");
        }

        // eth_dev_info_t ethernet_init_get_dev_info(esp_eth_handle_t *eth_handle)
        // {
        //     eth_dev_info_t ret = {.type = ETH_DEV_TYPE_UNKNOWN};

        //     for (int i = 0; i < eth_cnt_g; i++) {
        //         if (eth_handle == eth_instance_g[i].eth_handle) {
        //             return eth_instance_g[i].dev_info;
        //         }
        //     }

        //     return ret;
        // }

        void EthAdapter::stop()
        {
            Publisher<ObjectDataSendToMetro>::publish(
                ObjectDataSendToMetro(
                    TYPE_INTERNAL_ETH,
                    STOP_ETH,
                    {}));
        }
        void EthAdapter::reconnect()
        {
            Publisher<ObjectDataSendToMetro>::publish(
                ObjectDataSendToMetro(
                    TYPE_INTERNAL_ETH,
                    RECONNECT_ETH,
                    {}));
        }
    }
}