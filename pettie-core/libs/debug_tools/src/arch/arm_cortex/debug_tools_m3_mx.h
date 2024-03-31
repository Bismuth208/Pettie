/**
 * @file debug_tools_m3_mx.h
 * @brief This file provide includes of various debug outputs
 *        Common include file for hefty things to make life simpler ;)
 *
 *        All of the prints are done asynchronously to the output stream.
 *        To enable specific printouts just add them to the debug_tools_conf.h in your project
 */
#ifndef _DEBUG_TOOLS_M3_MX_H
#define _DEBUG_TOOLS_M3_MX_H

#ifdef __cplusplus
extern "C" {
#endif

// includes main sdkconfig.h file from build and main project
#include "debug_tools_conf.h"

#if (CONFIG_ASYNC_PRINTF_USE_RTOS == 1)
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "event_groups.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#endif // CONFIG_ASYNC_PRINTF_USE_RTOS

// ----------------------------------------------------------------------
// Definitions, type & enum declaration

#define CONFIG_DEFAULT_CPU_FREQ_MHZ configCPU_CLOCK_HZ

#if ((CONFIG_SYS_STATS_DBG_PRINTOUT == 1) && (configGENERATE_RUN_TIME_STATS == 0))
#error "Please enable configGENERATE_RUN_TIME_STATS in FreeRTOSConfig.h !
#endif

typedef uint32_t profile_time_t;

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

#endif // _DEBUG_TOOLS_M3_MX_H