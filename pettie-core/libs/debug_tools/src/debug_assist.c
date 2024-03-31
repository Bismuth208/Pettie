#include "debug_assist.h"

#include "debug_tools.h"

//
#include <assert.h>
#include <stdint.h>
#include <string.h>

// ----------------------------------------------------------------------
// Definitions, type & enum declaration


// ----------------------------------------------------------------------
// FreeRTOS Variables

#if((CONFIG_ENABLE_DEBUG_TOOLS == 1) && (CONFIG_SYS_STATS_DBG_PRINTOUT == 1))
TaskStatus_t* pxTaskStatusArray = NULL;
//
TimerHandle_t xSysStatsPlotterTimer = NULL;
#if (configSUPPORT_STATIC_ALLOCATION == 1)
StaticTimer_t xSysStatsPlotterTimerControlBlock;
#endif
#endif // CONFIG_ENABLE_DEBUG_TOOLS && CONFIG_SYS_STATS_DBG_PRINTOUT


// ----------------------------------------------------------------------
// Variables

#if((CONFIG_ENABLE_DEBUG_TOOLS == 1) && (CONFIG_SYS_STATS_DBG_PRINTOUT == 1))
uint8_t ucWriteBuffer[CONFIG_SYS_STATS_BUF_SIZE] = {0u};
volatile UBaseType_t uxArraySizeAllocated = 0u;
#endif // CONFIG_ENABLE_DEBUG_TOOLS && CONFIG_SYS_STATS_DBG_PRINTOUT


// ----------------------------------------------------------------------
// Static functions declaration

#if((CONFIG_ENABLE_DEBUG_TOOLS == 1) && (CONFIG_SYS_STATS_DBG_PRINTOUT == 1))

/**
 * @brief
 */ 
static void init_debug_assist_rtos(void);

/**
 * @brief
 */ 
static void vTaskStatsAlloc(void);


/**
 * @brief
 * 
 * 
 * @example:
 *      Task name    Runtime       CPU   Core  Prior.
 *       Tmr Svc     122809      <1 %      0       1 
 *   task_printf    3371394       4 %      0       1 
 *          IDLE   70442570      99 %      1       0 
 *          IDLE   65161706      92 %      0       0 
 *       ui_core    1955697       2 %      0       2 
 *       sys_evt        251      <1 %      0      20 
 *     esp_timer       8419      <1 %      0      22 
 *       emac_rx      16246      <1 %      0      15 
 *   now_if_task      48951      <1 %      0       3 
 *   eth_if_task      14354      <1 %      0       2 
 *          wifi     347508      <1 %      1      23 
 *          ipc0      20518      <1 %      0      24 
 *          ipc1      28566      <1 %      1      24
 */
static void vGetSysStats(char* pcWriteBuffer, size_t xMaxWriteBufferLen);

/**
 * @brief
 */ 
static void debug_sys_stats_plotter_timer(void *pvArg);
#endif // CONFIG_ENABLE_DEBUG_TOOLS && CONFIG_SYS_STATS_DBG_PRINTOUT


// ----------------------------------------------------------------------
// Static functions

#if((CONFIG_ENABLE_DEBUG_TOOLS == 1) && (CONFIG_SYS_STATS_DBG_PRINTOUT == 1))
static void
vTaskStatsAlloc(void)
{
	uxArraySizeAllocated = uxTaskGetNumberOfTasks();
	pxTaskStatusArray = pvPortMalloc(uxArraySizeAllocated * sizeof(TaskStatus_t));

	assert(pxTaskStatusArray);
}

static void
vGetSysStats(char* pcWriteBuffer, size_t xMaxWriteBufferLen)
{
	volatile UBaseType_t uxArraySize = 0u;
	volatile UBaseType_t x;
	unsigned long ulTotalRunTime;
	unsigned long ulStatsAsPercentage;
	int iStat = -1;

	// Make sure the write buffer does not contain a string.
	*pcWriteBuffer = 0x00;

	if (pxTaskStatusArray != NULL) {
		// Generate raw status information about each task.
		uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySizeAllocated, &ulTotalRunTime);
		ulTotalRunTime /= 100UL;

		if (ulTotalRunTime > 0u) {
			iStat = snprintf(pcWriteBuffer,
			                 xMaxWriteBufferLen,
#ifdef HAS_MULTICORE_SUPPORT
			                 "\n%16s %10s %9s %6s %7s\n",
#else
							         "\n%16s %10s %9s %7s\n",
#endif
			                 "Task name",
			                 "Runtime",
			                 "CPU",
#ifdef HAS_MULTICORE_SUPPORT
							 				 "Core",
#endif
			                 "Prior.");
			if ((iStat >= 0) && (iStat < xMaxWriteBufferLen)) {
				pcWriteBuffer += strnlen((char*)pcWriteBuffer, xMaxWriteBufferLen);
			}

			for (x = 0; x < uxArraySize; x++) {
				ulStatsAsPercentage = pxTaskStatusArray[x].ulRunTimeCounter / ulTotalRunTime;

				if (ulStatsAsPercentage > 0UL) {
					iStat = snprintf(pcWriteBuffer,
					                 xMaxWriteBufferLen,
#ifdef HAS_MULTICORE_SUPPORT
					                 "%16s %10u %7u %s  %5d %7d \n",
#else
									         "%16s %10u %7u %s  %7d \n",
#endif
					                 pxTaskStatusArray[x].pcTaskName,
					                 (unsigned int)pxTaskStatusArray[x].ulRunTimeCounter,
					                 (unsigned int)ulStatsAsPercentage,
					                 "%%",
#ifdef HAS_MULTICORE_SUPPORT
													 pxTaskStatusArray[x].xCoreID,
#endif
					                 pxTaskStatusArray[x].uxBasePriority);
				} else {
					iStat = snprintf(pcWriteBuffer,
					                 xMaxWriteBufferLen,
#ifdef HAS_MULTICORE_SUPPORT
					                 "%16s %10u %10s  %5d %7d \n",
#else
									 				 "%16s %10u %10s  %7d \n",
#endif
					                 pxTaskStatusArray[x].pcTaskName,
					                 (unsigned int)pxTaskStatusArray[x].ulRunTimeCounter,
					                 "<1 %%",
#ifdef HAS_MULTICORE_SUPPORT
													 pxTaskStatusArray[x].xCoreID,
#endif
					                 pxTaskStatusArray[x].uxBasePriority);
				}

				if ((iStat >= 0) && (iStat < xMaxWriteBufferLen)) {
					pcWriteBuffer += strnlen((char*)pcWriteBuffer, xMaxWriteBufferLen);
				} else {
					break;
				}
			}
		}
	}
}
#endif // CONFIG_ENABLE_DEBUG_TOOLS && CONFIG_SYS_STATS_DBG_PRINTOUT


static void
init_debug_assist_rtos(void)
{
#if((CONFIG_ENABLE_DEBUG_TOOLS == 1) && (CONFIG_SYS_STATS_DBG_PRINTOUT == 1))
#if (configSUPPORT_STATIC_ALLOCATION == 1)
	xSysStatsPlotterTimer = xTimerCreateStatic("SysStatsPlot",
	                                           pdMS_TO_TICKS(CONFIG_SYS_STATS_PLOT_TIMEOUT),
	                                           pdTRUE,
	                                           NULL,
	                                           (TimerCallbackFunction_t)(debug_sys_stats_plotter_timer),
	                                           &xSysStatsPlotterTimerControlBlock);
#else
	xSysStatsPlotterTimer = xTimerCreate("SysStatsPlot",
											pdMS_TO_TICKS(CONFIG_SYS_STATS_PLOT_TIMEOUT),
											pdTRUE,
											NULL,
											(TimerCallbackFunction_t)(debug_sys_stats_plotter_timer));
#endif

	assert(xSysStatsPlotterTimer);
#endif // CONFIG_ENABLE_DEBUG_TOOLS && CONFIG_SYS_STATS_DBG_PRINTOUT
}

// ----------------------------------------------------------------------
// Accessors functions

void
debug_assist_start(void)
{
#if((CONFIG_ENABLE_DEBUG_TOOLS == 1) && (CONFIG_SYS_STATS_DBG_PRINTOUT == 1))
	vTaskStatsAlloc();
	xTimerStart(xSysStatsPlotterTimer, pdMS_TO_TICKS(CONFIG_SYS_STATS_PLOT_TIMEOUT));
#endif // CONFIG_ENABLE_DEBUG_TOOLS && CONFIG_SYS_STATS_DBG_PRINTOUT
}

// ----------------------------------------------------------------------
// FreeRTOS functions

#if((CONFIG_ENABLE_DEBUG_TOOLS == 1) && (CONFIG_SYS_STATS_DBG_PRINTOUT == 1))
static void
debug_sys_stats_plotter_timer(void *pvArg)
{
	(void) pvArg;

	vGetSysStats((char*)&ucWriteBuffer[0], sizeof(ucWriteBuffer));
	// This print must be always enabled with no options to configure it!
	async_printf(async_print_type_str, (const char*)&ucWriteBuffer[0], 0);
}
#endif // CONFIG_ENABLE_DEBUG_TOOLS && CONFIG_SYS_STATS_DBG_PRINTOUT

// ----------------------------------------------------------------------
// Core functions

void
init_debug_assist(void)
{
	init_debug_assist_rtos();
}