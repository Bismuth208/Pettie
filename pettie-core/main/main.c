//
#include "common/common.h"
#include "common/hash_functions.h"
#include "common/pins.h"
#include "common/settings.h"
#include "servo/servo.h"
#include "pettie_core.h"
//
#include "mqtt/mqtt_func.h"
#include "wireless/wireless.h"
//
#include "debug_tools.h"
//
#include "sdkconfig.h"
//
#include "freertos/FreeRTOS.h"
#include "freertos/FreeRTOSConfig.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "freertos/timers.h"

//
#include "driver/gpio.h"
#include "esp_attr.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
//
#include <assert.h>
#include <stdint.h>
#include <string.h>

// ----------------------------------------------------------------------
// Definitions, type & enum declaration


// ----------------------------------------------------------------------
// FreeRTOS Variables

// For Task syncronization
EventGroupHandle_t xTaskSyncEventGroupHandler;
StaticEventGroup_t xTaskSyncBlockEventGroup;

// ----------------------------------------------------------------------
// Variables


// ----------------------------------------------------------------------
// Static functions declaration

static void init_main_rtos(void);

// ----------------------------------------------------------------------
// Static functions


static void init_main_rtos(void)
{
    xTaskSyncEventGroupHandler = xEventGroupCreateStatic(&xTaskSyncBlockEventGroup);
    assert(xTaskSyncEventGroupHandler);
}

void init_network_misc(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
}

void init_gpio(void)
{
  gpio_set_direction(GENTLE_TOUCH_TAIL_PIN, GPIO_MODE_INPUT);
	gpio_set_pull_mode(GENTLE_TOUCH_TAIL_PIN, GPIO_PULLUP_ONLY);
}

// ----------------------------------------------------------------------
// Accessors functions

void task_sync_set_bits(uint32_t ulBits)
{
    xEventGroupSetBits(xTaskSyncEventGroupHandler, ulBits);
}

void task_sync_get_bits(uint32_t ulBits)
{
    xEventGroupWaitBits(xTaskSyncEventGroupHandler, ulBits, pdTRUE, pdFALSE, portMAX_DELAY);
}

// ----------------------------------------------------------------------
void app_main(void)
{
    init_debug_tools();
    init_memory_model();

    init_main_rtos();
    init_gpio();
    init_network_misc();
    init_wifi();
    init_mqtt();

    init_servo();
    init_pettie_core();

    // --------------------------
    // start everything now safely
    debug_tools_start();

    task_sync_set_bits(TASK_SYNC_EVENT_BIT_ALL);

    // I don't want to play with you anymore
    vTaskDelete(NULL);
}
