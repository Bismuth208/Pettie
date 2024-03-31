#include "common/common.h"
#include "mqtt/mqtt_func.h"
#include "wireless.h"
#include "wireless_conf.h"
//
#include "debug_tools.h"
#include "memory_model/memory_model.h"

//
#include "sdkconfig.h"
//
#include <freertos/FreeRTOS.h>
#include <freertos/FreeRTOSConfig.h>
#include <freertos/event_groups.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <freertos/timers.h>
//
#include <esp_attr.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_timer.h>
#include <esp_wifi.h>
#include <nvs_flash.h>
//
#include <assert.h>
#include <stdint.h>
#include <string.h>


// ----------------------------------------------------------------------
// Definitions, type & enum declaration

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

// ----------------------------------------------------------------------
// FreeRTOS Variables

const char* assigned_name_for_wifi_rssi   = "mqtt_pub";
TimerHandle_t xWiFiUpdateRRSiTimerHandler = NULL;
StaticTimer_t xWiFiUpdateRRSiTimerControlBlock;

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

// ----------------------------------------------------------------------
// Variables

static const char* TAG = "wifi";

// Add the next variables with your SSID/Password combination
wifi_network_t wifi_networks[] = {
    {WIFI_SSID_NAME,          WIFI_SSID_PASSWORD          }
};
_Static_assert((sizeof(wifi_networks) / sizeof(wifi_networks[0])) == (MAX_WIFI_NETWORKS), "Wi-Fi networks count mismatch!");

wifi_config_t wifi_config = {
    .sta = {
        /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (pasword len => 8).
          * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
          * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
          * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
          */
        .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        .sae_h2e_identifier = "",
    },
};

esp_event_handler_instance_t instance_any_id;
esp_event_handler_instance_t instance_got_ip;

///
size_t current_wifi_network = 0u;
static int s_retry_num      = 0;

int iLinkRSSI = -98;


// ----------------------------------------------------------------------
// Static functions declaration

static void wifi_set_tx_power(int8_t ic_new_tx_power);

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);


static void wifi_update_rssi_timer(void* pvArg);

/**
 * @brief Creates FreeRTOS objects what need to maintain WiFi.
 */
static void init_wifi_rtos(void);

static void wifi_init_sta(void);

// ----------------------------------------------------------------------
// Static functions

static void wifi_set_tx_power(int8_t ic_new_tx_power)
{
    // Convert range
    // [WIFI_MIN_TX_POWER_PERCENTAGE : WIFI_MAX_TX_POWER_PERCENTAGE]
    // to
    // [WIFI_MIN_TX_POWER, WIFI_MAX_TX_POWER]
    int8_t ic_tx_Power = ul_map_val(ic_new_tx_power, WIFI_MIN_TX_POWER_PERCENTAGE, WIFI_MAX_TX_POWER_PERCENTAGE, WIFI_MIN_TX_POWER, WIFI_MAX_TX_POWER);
    ESP_ERROR_CHECK(esp_wifi_set_max_tx_power(ic_tx_Power));
}

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if ((event_base == WIFI_EVENT) && (event_id == WIFI_EVENT_STA_START))
    {
        esp_wifi_connect();
    }
    else if ((event_base == WIFI_EVENT) && (event_id == WIFI_EVENT_STA_DISCONNECTED))
    {
        if (s_retry_num < (3 * MAX_WIFI_NETWORKS + 1))
        {
            s_retry_num++;

            ESP_ERROR_CHECK(esp_wifi_stop());

            ++current_wifi_network;
            if (current_wifi_network >= MAX_WIFI_NETWORKS)
            {
                current_wifi_network = 0u;
            }

            strncpy((char*)wifi_config.sta.ssid, wifi_networks[current_wifi_network].ssid, sizeof(wifi_config.sta.ssid) - 1);
            strncpy((char*)wifi_config.sta.password, wifi_networks[current_wifi_network].password, sizeof(wifi_config.sta.password) - 1);

            ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
            ESP_ERROR_CHECK(esp_wifi_start());
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        // ip_event_got_ip_t* event           = (ip_event_got_ip_t*)event_data;
        // const esp_netif_ip_info_t* ip_info = &event->ip_info;

        // ESP_LOGI(TAG, "WiFi Got IP Address");
        // ESP_LOGI(TAG, "~~~~~~~~~~~");
        // ESP_LOGI(TAG, "ETHIP:" IPSTR, IP2STR(&ip_info->ip));
        // ESP_LOGI(TAG, "ETHMASK:" IPSTR, IP2STR(&ip_info->netmask));
        // ESP_LOGI(TAG, "ETHGW:" IPSTR, IP2STR(&ip_info->gw));
        // ESP_LOGI(TAG, "~~~~~~~~~~~");

        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static void init_wifi_rtos(void)
{
    xWiFiUpdateRRSiTimerHandler = xTimerCreateStatic(assigned_name_for_wifi_rssi,
                                                     pdMS_TO_TICKS(WIFI_RSSI_UPDATE_TIMEOUT),
                                                     pdTRUE,
                                                     NULL,
                                                     (TimerCallbackFunction_t)(wifi_update_rssi_timer),
                                                     &xWiFiUpdateRRSiTimerControlBlock);
    assert(xWiFiUpdateRRSiTimerHandler);

    s_wifi_event_group = xEventGroupCreate();
    assert(s_wifi_event_group);
}

static void wifi_init_sta(void)
{
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));

    strncpy((char*)wifi_config.sta.ssid, wifi_networks[current_wifi_network].ssid, sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char*)wifi_config.sta.password, wifi_networks[current_wifi_network].password, sizeof(wifi_config.sta.password) - 1);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    // ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_MIN_MODEM));

    ESP_ERROR_CHECK(esp_wifi_start());

    wifi_set_tx_power(DEFAULT_WIFI_TX_POWER);

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
    * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    (void)xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
}

// ----------------------------------------------------------------------
// Accessors functions


// ----------------------------------------------------------------------
// FreeRTOS functions

static void wifi_update_rssi_timer(void* pvArg)
{
    (void)pvArg;

    esp_wifi_sta_get_rssi(&iLinkRSSI);
    memory_model_set_i32(TOPIC_ID_RSSI_VAL, (int32_t)iLinkRSSI);
}

// ----------------------------------------------------------------------
// Core functions

void init_wifi()
{
    init_wifi_rtos();
    wifi_init_sta();

    xTimerStart(xWiFiUpdateRRSiTimerHandler, WIFI_RSSI_UPDATE_TIMEOUT);
}
