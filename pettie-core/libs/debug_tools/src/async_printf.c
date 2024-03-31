#include "async_printf.h"

#include "async_printf_conf.h"
#include "arch/debug_tools_arch.h"

//
#include <assert.h>
#include <stdio.h>
#include <string.h>

// ----------------------------------------------------------------------
// Definitions, type & enum declaration

/// Create bit mask, which will be used as "pass" range and filter overflow
#define ASYNC_PRINTF_BUFFER_MASK (CONFIG_ASYNC_PRINTF_MAX_ITEMS - 1u)

typedef struct
{
	size_t write_index;
	size_t read_index;
	async_print_item_t items[CONFIG_ASYNC_PRINTF_MAX_ITEMS];
} async_printf_ring_buffer_t;

// ----------------------------------------------------------------------
// FreeRTOS Variables

#if ((CONFIG_ENABLE_DEBUG_TOOLS == 1) && (CONFIG_ASYNC_PRINTF_USE_RTOS == 1))

// Print all stuff to console
const char* assigned_name_for_task_printf = CONFIG_ASSIGNED_TASK_PRINTF_NAME;
TaskHandle_t xPrintfTaskHandler = NULL;

#if (configSUPPORT_STATIC_ALLOCATION == 1)
StaticTask_t xPrintfTaskControlBlock;
StackType_t xPrintfStack[CONFIG_STACK_WORDS_SIZE_FOR_TASK_PRINTF];
#endif
#endif // CONFIG_ASYNC_PRINTF_USE_RTOS

// ----------------------------------------------------------------------
// Variables

static volatile async_printf_ring_buffer_t async_printf_buffer = {0u};

uint8_t async_printf_fmt_buf[CONFIG_ASYNC_PRINTF_MAX_OUTPUT_BUF_LEN] = {0u};

// ----------------------------------------------------------------------
// Static functions declaration

static void init_async_printf_rtos(void);

#if ((CONFIG_ENABLE_DEBUG_TOOLS == 1) && (CONFIG_ASYNC_PRINTF_USE_RTOS == 1))
static void vPrintfTask(void* pvArg);
#endif // CONFIG_ASYNC_PRINTF_USE_RTOS

/**
 * @brief Print items from the circular buffer as one item per call of function.
 */
static void async_printf_print(void);

// ----------------------------------------------------------------------
// Static functions

static void
async_printf_print(void)
{
	volatile async_print_item_t* local_tail = &async_printf_buffer.items[async_printf_buffer.read_index];
	async_printf_buffer.read_index = (async_printf_buffer.read_index + 1u) & ASYNC_PRINTF_BUFFER_MASK;

	// Use local varialbe to increase Register/Cache data locality
	async_print_type_t type = local_tail->type;
	uint32_t value = local_tail->value;
	const char* async_msg = (const char*) local_tail->msg;

	switch(type)
	{
	case async_print_type_str: {
		// local_tail->type = async_print_type_MAX;
		snprintf((char*)&async_printf_fmt_buf, CONFIG_ASYNC_PRINTF_MAX_OUTPUT_BUF_LEN, async_msg);
		break;
	}

	case async_print_type_u32: {
		// local_tail->type = async_print_type_MAX;
		snprintf((char*)&async_printf_fmt_buf, CONFIG_ASYNC_PRINTF_MAX_OUTPUT_BUF_LEN, async_msg, (uint32_t)value);
		break;
	}

	case async_print_type_i32: {
		// local_tail->type = async_print_type_MAX;
		snprintf((char*)&async_printf_fmt_buf, CONFIG_ASYNC_PRINTF_MAX_OUTPUT_BUF_LEN, async_msg, (int32_t)value);
		break;
	}

	default:
		break;
	}

	puts((const char*)&async_printf_fmt_buf);
}


static void
init_async_printf_rtos(void)
{
#if ((CONFIG_ENABLE_DEBUG_TOOLS == 1) && (CONFIG_ASYNC_PRINTF_USE_RTOS == 1))
#if ((configNUM_CORES > 1) && (configUSE_CORE_AFFINITY == 1))
	xPrintfTaskHandler = xTaskCreateStaticPinnedToCore((TaskFunction_t)(vPrintfTask),
	                                                    assigned_name_for_task_printf,
	                                                    CONFIG_STACK_WORDS_SIZE_FOR_TASK_PRINTF,
	                                                    NULL,
	                                                    CONFIG_PRIORITY_LEVEL_FOR_TASK_PRINTF,
	                                                    xPrintfStack,
	                                                    &xPrintfTaskControlBlock,
	                                                    (BaseType_t)ASYNC_PRINTF_CORE);
#else

#if (configSUPPORT_STATIC_ALLOCATION == 1)
	xPrintfTaskHandler = xTaskCreateStatic((TaskFunction_t)(vPrintfTask),
												assigned_name_for_task_printf,
												CONFIG_STACK_WORDS_SIZE_FOR_TASK_PRINTF,
												NULL,
												CONFIG_PRIORITY_LEVEL_FOR_TASK_PRINTF,
												xPrintfStack,
												&xPrintfTaskControlBlock);
#else
	xTaskCreate((TaskFunction_t)(vPrintfTask),
					assigned_name_for_task_printf,
					CONFIG_STACK_WORDS_SIZE_FOR_TASK_PRINTF,
					NULL,
					CONFIG_PRIORITY_LEVEL_FOR_TASK_PRINTF,
					&xPrintfTaskHandler);
#endif
#endif

	assert(xPrintfTaskHandler);
#endif // CONFIG_ASYNC_PRINTF_USE_RTOS
}

// ----------------------------------------------------------------------
// Accessors functions

void IRAM_ATTR
async_printf(async_print_type_t item_type, const char* new_msg, uint32_t new_value)
{
	// Get current head position and move it as fast as possible
	volatile async_print_item_t* local_head = &async_printf_buffer.items[async_printf_buffer.write_index];
	async_printf_buffer.write_index = (async_printf_buffer.write_index + 1u) & ASYNC_PRINTF_BUFFER_MASK;

	local_head->type = item_type;
	local_head->msg = new_msg;
	local_head->value = new_value;
}

void IRAM_ATTR
async_printf_sync(void)
{
	if(async_printf_buffer.read_index != async_printf_buffer.write_index) {
		async_printf_print();
	}
}

// ----------------------------------------------------------------------
// FreeRTOS functions

#if ((CONFIG_ENABLE_DEBUG_TOOLS == 1) && (CONFIG_ASYNC_PRINTF_USE_RTOS == 1))
static void
vPrintfTask(void* pvArg)
{
	(void)pvArg;

	ASYNC_PRINTF(CONFIG_ENABLE_TASK_START_EVENT_DBG_PRINTOUT, async_print_type_str, assigned_name_for_task_printf, 0);

	for(;;) {
		async_printf_sync();
		vTaskDelay(pdMS_TO_TICKS(CONFIG_SYNC_PERIOD_TASK_PRINTF));
	}
}
#endif // CONFIG_ASYNC_PRINTF_USE_RTOS

// ----------------------------------------------------------------------
// Core functions

void init_async_printf(void)
{
	init_async_printf_rtos();
}