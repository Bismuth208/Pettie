/**
 * @file debug_tools_m3_mx.c
 *  
 *
 * @note Result values may have delta up to +/-6 instructions !
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

#if (defined(CORTEX) || defined(ARM))

#include "debug_tools_m3_mx.h"

#include <stdint.h>
#include <string.h>

// ----------------------------------------------------------------------
// Definitions, type & enum declaration

typedef struct {
  uint32_t DWT_LAR;        // Lock Access Register  | 0xE0000FB0
  uint32_t DWT_LSR;        // Lock Status Register  | 0xE0000FB4

  uint32_t DWT_UNKNOWN[18];

  uint32_t DWT_CTRL;       // Control Register      | 0xE0001000
  uint32_t DWT_CYCCNT;     // Cycle Count Register  | 0xE0001004
  uint32_t DWT_CPICNT;     // CPI Count Register
  uint32_t DWT_EXCCNT;     // Exception Overhead Count Register
  uint32_t DWT_SLEEPCNT;   // Sleep Count Register
  uint32_t DWT_LSUCNT;     // LSU Count Register
  uint32_t DWT_FOLDCNT;    // Folded-instruction Count Register
  uint32_t DWT_PCSR;       // Program Counter Sample Register
  uint32_t DWT_COMP0;      // Comparator Register 0
  uint32_t DWT_MASK0;      // Mask Register 0
  uint32_t DWT_FUNCTION0;  // Function Register 0
  uint32_t DWT_COMP1;      // Comparator Register 1
  uint32_t DWT_MASK1;      // Mask Register 1
  uint32_t DWT_FUNCTION1;  // Function Register 1
  uint32_t DWT_COMP2;      // Comparator Register 2
  uint32_t DWT_MASK2;      // Mask Register 2
  uint32_t DWT_FUNCTION2;  // Function Register 2
  uint32_t DWT_COMP3;      // Comparator Register 3
  uint32_t DWT_MASK3;      // Mask Register 3
  uint32_t DWT_FUNCTION3;  // Function Register 3
  // Some of the registers is not added here
  // From 0xE0001FD0 -> 0xE0001FFC
} DWT_TypeDef;  // 0xE0000FB0 -> 0xE0000FB4

/*
 * ITM registers to perform clocks count
 * For most of Cortex M3/M4 and others this registers are the same (mostly ?).
 */
#define SCB_DEMCR   *(volatile uint32_t* )0xE000EDFC // CoreDebug
#define DWT         ((DWT_TypeDef*) ((volatile uint32_t) 0xE0000FB0))


// ----------------------------------------------------------------------
// Variables

const uint32_t DWT_LAR_MAGIC = 0xC5ACCE55;

void dwt_enable(void);
void dwt_counter_reset(void);
uint32_t dwt_counter_value_get(void);


// ----------------------------------------------------------------------
// Accessors functions

/*
 * @brief Enables debug counter to mesure wasted clocks
 * @retval none
 */
void
dwt_enable(void)
{
  DWT->DWT_LAR = DWT_LAR_MAGIC;  // unlock access to DWT (ITM, etc.)registers
  SCB_DEMCR |= 0x01000000;       // enable trace

  DWT->DWT_CTRL |= 1;         // enable the counter
  DWT->DWT_CYCCNT = 0;        // reset the counter
}


void
dwt_counter_reset(void)
{
  DWT->DWT_CYCCNT = 0;        // reset the counter
}

/**
 * @brief
 * 
 * @note Value in this register overflows pretty darn fast! 
 */
uint32_t
dwt_counter_value_get(void)
{
  return DWT->DWT_CYCCNT;
}


profile_time_t
profiler_get_us_time(void)
{
  // This will not work with dynamicly changed Frequency at runtime!
  return (profile_time_t)(dwt_counter_value_get() / CONFIG_DEFAULT_CPU_FREQ_MHZ);
}

// ----------------------------------------------------------------------
// Core functions

void
init_debug_tools_arch(void)
{
  dwt_enable();
}

#endif // ARM || CORTEX