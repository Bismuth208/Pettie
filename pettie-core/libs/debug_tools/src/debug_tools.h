/**
 * @file debug_tools.h
 * @brief This file provide includes of various debug outputs
 *        Common include file for hefty things to make life simpler ;)
 *
 *        All of the prints are done asynchronously to the output stream.
 *        To enable specific printouts just check Kconfig:
 *          - Component config -> Debug assistant configuration ->
 *          - Component config -> Async printf configuration ->
 */
#ifndef _DEBUG_TOOLS_H
#define _DEBUG_TOOLS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "arch/debug_tools_arch.h"

#include "async_printf.h"
#include "async_printf_conf.h"
#include "async_profiler.h"
#include "debug_assist.h"

// ----------------------------------------------------------------------
// Core functions

/**
 * @brief Call initialisation for debug functions
 */ 
void init_debug_tools(void);

/**
 * @brief Provide signal to launch any debug functions
 * 
 * @note Must be called after all Tasks, objects and etc are created!
 */ 
void debug_tools_start(void);

#ifdef __cplusplus
}
#endif

#endif // _DEBUG_TOOLS_H