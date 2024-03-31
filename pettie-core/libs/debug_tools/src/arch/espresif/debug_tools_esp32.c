/**
 * @file debug_tools_esp32.c
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

#if (defined(ESP32) || defined(ESP_PLATFORM))

#include "debug_tools_esp32.h"

//
#include "xtensa/hal.h"

// ----------------------------------------------------------------------
// Accessors functions

profile_time_t
profiler_get_us_time(void)
{
  // This will not work with dynamicly changed Frequency at runtime!
  return (profile_time_t)(xthal_get_ccount() / CONFIG_DEFAULT_CPU_FREQ_MHZ);
}

// ----------------------------------------------------------------------
// Core functions

void
init_debug_tools_arch(void)
{
  // Nothing to implement here everything is already done in SDK
}

#endif // ESP32 || ESP_PLATFORM