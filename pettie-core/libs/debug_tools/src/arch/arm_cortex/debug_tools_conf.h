/**
 * @file debug_tools_conf.h
 * 
 * @brief This file provide control of various debug outputs
 * 
 * All of the prints are done asynchronously to the output stream.
 * To enable specific printouts just uncomment required defines below.
 */

#ifndef _DEBUG_TOOLS_CONF_H
#define _DEBUG_TOOLS_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

// Enable/disable whole debug output
#define CONFIG_ENABLE_DEBUG_TOOLS (0)

#define CONFIG_ASYNC_PRINTF_MAX_ITEMS          (512)
#define CONFIG_ASYNC_PRINTF_MAX_OUTPUT_BUF_LEN (2048)
#define CONFIG_PROFILER_POINTS_MAX             (32)

#define CONFIG_ASYNC_PRINTF_USE_RTOS            (1)
#define CONFIG_ASSIGNED_TASK_PRINTF_NAME        ("task_printf")
#define CONFIG_STACK_WORDS_SIZE_FOR_TASK_PRINTF (1024)
#define CONFIG_PRIORITY_LEVEL_FOR_TASK_PRINTF   (TASK_PRIORITIES_LOW)
#define CONFIG_SYNC_PERIOD_TASK_PRINTF          (10)

// Enable/disable FreeRTOS stats print
#define CONFIG_SYS_STATS_DBG_PRINTOUT  (0)
#define CONFIG_SYS_STATS_BUF_SIZE      (8192)
#define CONFIG_SYS_STATS_PLOT_TIMEOUT  (1000)


#ifdef CONFIG_ENABLE_DEBUG_TOOLS


// ---------------------------------
// Uncomment defines below to enable logs:
//  - with general debug prints
//
// Naming rule:
//  - #define YOUR_NAME##_DBG_PRINTOUT

// Add defines here


// ---------------------------------
// Uncomment defines below to enable logs:
//  - with profile debug prints
// Note: only one at time should be uncommented!  
//
// Profiler point Id's
//
// NOTE: Point Id Should not be higher than @ref PROFILER_POINTS_MAX
//
// Naming rule:
//  - #define YOUR_NAME##_DBG_PROFILER
//  - #define YOUR_NAME##_DBG_PROFILER_POINT_ID (someMagicNumberId)

// Add defines here

#ifdef __cplusplus
}
#endif
#endif // ENABLE_DEBUG_TOOLS
#endif // _DEBUG_TOOLS_CONF_H
