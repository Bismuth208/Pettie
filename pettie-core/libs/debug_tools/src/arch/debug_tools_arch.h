/**
 * @file debug_tools_arch.h
 * @brief This file provide architecture specific includes
 * 
 * @author Alexandr Antonov (@Bismuth208)
 *
 * Format:
 *  1 Tab == 2 spaces
 *  UTF-8
 *  EOL - Unix
 *
 * Lang: C
 * Prefered Compiler: GCC
 *
 * Licence: MIT
 */
#ifndef _DEBUG_TOOLS_ARCH_H
#define _DEBUG_TOOLS_ARCH_H

#ifdef __cplusplus
extern "C" {
#endif

#if (defined(ESP32) || defined(ESP_PLATFORM))

#define HAS_MULTICORE_SUPPORT

#include "espresif/debug_tools_esp32.h"
#else
#define IRAM_ATTR
#endif

#if (defined(CORTEX) || defined(ARM))
#include "arm_cortex/debug_tools_m3_mx.h"
#endif

// ----------------------------------------------------------------------
// Core functions

/**
 * @brief Initialise Architecture specific registers and stuff 
 */
void init_debug_tools_arch(void);

#ifdef __cplusplus
}
#endif

#endif // _DEBUG_TOOLS_ARCH_H