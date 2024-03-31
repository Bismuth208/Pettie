/**
 * @file debug_tools_esp32.h
 * @brief This file provide includes of various debug outputs
 *        Common include file for hefty things to make life simpler ;)
 *
 *        All of the prints are done asynchronously to the output stream.
 *        To enable specific printouts just them in Kconfig in your project and enable with menuconfig:
 *          - Debug tools config -> Debug assistant configuration ->
 *          - Debug tools config -> Async printf configuration ->
 */
#ifndef _DEBUG_TOOLS_ESP32_H
#define _DEBUG_TOOLS_ESP32_H

#ifdef __cplusplus
extern "C" {
#endif

// includes main sdkconfig.h file from build and main project
#include "debug_tools_conf.h"

//
#include "esp_attr.h"

#if (CONFIG_ASYNC_PRINTF_USE_RTOS == 1)
#include "freertos/FreeRTOS.h"
#include "freertos/FreeRTOSConfig.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#endif // CONFIG_ASYNC_PRINTF_USE_RTOS

#include <stdint.h>
#include <stddef.h>

// ----------------------------------------------------------------------
// Definitions, type & enum declaration

#define CONFIG_DEFAULT_CPU_FREQ_MHZ CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ

#if ((CONFIG_SYS_STATS_DBG_PRINTOUT == 1) && (configGENERATE_RUN_TIME_STATS == 0))
#error "Please enable configGENERATE_RUN_TIME_STATS in menuconfig !"
#endif

#if (CONFIG_ASYNC_PRINTF_CORE0 == 1)
#define ASYNC_PRINTF_CORE 0
#endif

#if (CONFIG_ASYNC_PRINTF_CORE1 == 1)
#define ASYNC_PRINTF_CORE 1
#endif

typedef uint64_t profile_time_t;

// ----------------------------------------------------------------------
// Accessors functions

/**
 * @brief Take system Time in microseconds
 * 
 * @return current system time
 * 
 * @note You have to implement this one (Arch dependent)
*/
profile_time_t profiler_get_us_time(void);

#ifdef __cplusplus
}
#endif

#endif // _DEBUG_TOOLS_ESP32_H