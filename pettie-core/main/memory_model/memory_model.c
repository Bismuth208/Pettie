#include "memory_model.h"

#include "common/common.h"
#include "common/hash_functions.h"

//
#include "debug_tools.h"
//
#include <sdkconfig.h>
//
#include <freertos/FreeRTOS.h>
#include <freertos/FreeRTOSConfig.h>
#include <freertos/event_groups.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <freertos/timers.h>
//
#include <esp_attr.h>
#include <esp_timer.h>
//
#include <assert.h>
#include <stdint.h>
#include <string.h>


// ----------------------------------------------------------------------
// Definitions, type & enum declaration


// ----------------------------------------------------------------------
// FreeRTOS Variables

#if (CONFIG_MEMORY_MODEL_ENABLE_DEBUG == 1)
// in ms
#    define MM_REPORT_TIMEOUT (1000u)
const char* assigned_name_for_timer_report = "mm_report";
TimerHandle_t xMMReportTimer               = NULL;
StaticTimer_t xMMReportTimerControlBlock;
#endif

// ----------------------------------------------------------------------
// Variables

volatile size_t DRAM_ATTR lock_failure_count = 0;

// ----------------------------------------------------------------------
// Static functions declaration
#if (CONFIG_MEMORY_MODEL_ENABLE_DEBUG == 1)
static void memory_model_report_timer(void* pvArg);
#endif

// ----------------------------------------------------------------------
// Static functions

/**
 * @brief runtime check if provided id is valid.
 *        To prevent:
 *          - out of bounds access
 *          - wrong id assigment
 * 
 * @param [in] topic_id
 * 
 * @note To improve perfomance assert() function might be disabled in menuconfig
 */
static void IRAM_ATTR __attribute__((optimize("-O2")))
memory_model_validate_id(topic_id_t topic_id)
{
    (void)topic_id;

    // not used right now
    // also disabled with _NDEBUG_ flag
    // this will be future feature for production!
    // not being tested!
#if 0
  assert(topic_id < TOPIC_ID_MAX);
  assert(topics_db[topic_id].self_id == topic_id);
#endif
}

/**
 * @brief Various checks and initialisation
 */
static void memory_model_validate(void)
{
    topic_db_t* px_topic_db = NULL;
    topic_t* px_topic       = NULL;

    for (size_t i = 0u; i < TOPIC_ID_MAX; i++)
    {
        px_topic_db = &topics_db[i];
        px_topic    = px_topic_db->topic;

        // If you here check alignment with enum and array!
        // topic_id_t and topic_db_t must be 1:1 !
        assert(px_topic_db->self_id == i);

        // now get hash
        px_topic->data.text_len = strnlen(px_topic->data.text, TOPICS_LEN_MAX);
        px_topic->data.hash     = ul_get_hash(px_topic->data.text, px_topic->data.text_len);
    }
}

void memory_model_init_rtos(void)
{
#if (CONFIG_MEMORY_MODEL_ENABLE_DEBUG == 1)
    xMMReportTimer = xTimerCreateStatic(assigned_name_for_timer_report,
                                        pdMS_TO_TICKS(MM_REPORT_TIMEOUT),
                                        pdTRUE,
                                        NULL,
                                        (TimerCallbackFunction_t)(memory_model_report_timer),
                                        &xMMReportTimerControlBlock);
    assert(xMMReportTimer);
#endif
}

// ----------------------------------------------------------------------
// Accessors functions

void IRAM_ATTR __attribute__((optimize("-O2")))
memory_model_update(topic_t* px_topic, BaseType_t xPublish)
{
    px_topic->status.update_timestamp = system_get_us_time();
    px_topic->status.do_publish       = (bool)xPublish;
}

void IRAM_ATTR __attribute__((optimize("-O2")))
memory_model_set_float(topic_id_t topic_id, float value, size_t precision)
{
    memory_model_validate_id(topic_id);

    const char* str_precision = NULL;

    switch (precision)
    {
        case STR_2_FLOAT_DOT_2: {
            str_precision = "%.2f";
            break;
        }

        case STR_2_FLOAT_DOT_5: {
            str_precision = "%.5f";
            break;
        }

        case STR_2_FLOAT_DOT_7: {
            str_precision = "%.7f";
            break;
        }

        default: {
            str_precision = "%.2f";
            break;
        }
    }

    if (memory_model_lock(topic_id, portMAX_DELAY))
    {
        topic_t* px_topic         = topics_db[topic_id].topic;
        size_t ofs                = snprintf(&px_topic->data.value[0], TOPICS_DATA_LEN_MAX, str_precision, value);
        px_topic->data.value[ofs] = '\0';

        memory_model_update(px_topic, pdTRUE);
        memory_model_unlock(topic_id);
    }
}

void IRAM_ATTR __attribute__((optimize("-O2")))
memory_model_set_bool(topic_id_t topic_id, bool value)
{
    memory_model_validate_id(topic_id);

    if (memory_model_lock(topic_id, portMAX_DELAY))
    {
        topic_t* px_topic         = topics_db[topic_id].topic;
        size_t ofs                = snprintf(&px_topic->data.value[0], TOPICS_DATA_LEN_MAX, "%s", (value) ? "1" : "0");
        px_topic->data.value[ofs] = '\0';

        memory_model_update(px_topic, pdTRUE);
        memory_model_unlock(topic_id);
    }
}

void IRAM_ATTR __attribute__((optimize("-O2")))
memory_model_set_u32(topic_id_t topic_id, uint32_t value)
{
    memory_model_validate_id(topic_id);

    if (memory_model_lock(topic_id, portMAX_DELAY))
    {
        topic_t* px_topic         = topics_db[topic_id].topic;
        size_t ofs                = snprintf(&px_topic->data.value[0], TOPICS_DATA_LEN_MAX, "%u", value);
        px_topic->data.value[ofs] = '\0';

        memory_model_update(px_topic, pdTRUE);
        memory_model_unlock(topic_id);
    }
}

void IRAM_ATTR __attribute__((optimize("-O2")))
memory_model_set_i32(topic_id_t topic_id, int32_t value)
{
    memory_model_validate_id(topic_id);

    if (memory_model_lock(topic_id, portMAX_DELAY))
    {
        topic_t* px_topic         = topics_db[topic_id].topic;
        size_t ofs                = snprintf(&px_topic->data.value[0], TOPICS_DATA_LEN_MAX, "%d", (int)value);
        px_topic->data.value[ofs] = '\0';

        memory_model_update(px_topic, pdTRUE);
        memory_model_unlock(topic_id);
    }
}

void IRAM_ATTR __attribute__((optimize("-O2")))
memory_model_set_str(topic_id_t topic_id, const char* value)
{
    memory_model_validate_id(topic_id);

    if (memory_model_lock(topic_id, portMAX_DELAY))
    {
        topic_t* px_topic         = topics_db[topic_id].topic;
        size_t ofs                = snprintf(&px_topic->data.value[0], TOPICS_DATA_LEN_MAX, "%s", value);
        px_topic->data.value[ofs] = '\0';

        memory_model_update(px_topic, pdTRUE);
        memory_model_unlock(topic_id);
    }
}

bool IRAM_ATTR __attribute__((optimize("-O2")))
memory_model_get_bool(topic_id_t topic_id, TickType_t timeout)
{
    memory_model_validate_id(topic_id);

    bool ret_value = false;

    if (memory_model_lock(topic_id, timeout))
    {
        if (strcmp((const char*)topics_db[topic_id].topic->data.value, "1") == 0)
        {
            ret_value = true;
        }
        else
        {
            ret_value = false;
        }

        memory_model_unlock(topic_id);
    }

    return ret_value;
}

int32_t IRAM_ATTR __attribute__((optimize("-O2")))
memory_model_get_i32(topic_id_t topic_id, TickType_t timeout)
{
    memory_model_validate_id(topic_id);

    int32_t ret_value = 0;

    if (memory_model_lock(topic_id, timeout))
    {
        ret_value = atoi(topics_db[topic_id].topic->data.value);

        memory_model_unlock(topic_id);
    }

    return ret_value;
}

float IRAM_ATTR __attribute__((optimize("-O2")))
memory_model_get_float(topic_id_t topic_id, TickType_t timeout)
{
    memory_model_validate_id(topic_id);

    float ret_value = 0.0;

    if (memory_model_lock(topic_id, timeout))
    {
        ret_value = atof(topics_db[topic_id].topic->data.value);

        memory_model_unlock(topic_id);
    }

    return ret_value;
}

bool memory_model_is_same(topic_id_t topic_id, TickType_t timeout, const char* val)
{
    memory_model_validate_id(topic_id);

    bool ret_value = false;

    if (memory_model_lock(topic_id, timeout))
    {
        if (strcmp((const char*)topics_db[topic_id].topic->data.value, val) == 0)
        {
            ret_value = true;
        }
        else
        {
            ret_value = false;
        }

        memory_model_unlock(topic_id);
    }

    return ret_value;
}

// ----------------
BaseType_t IRAM_ATTR __attribute__((optimize("-O2")))
memory_model_lock(topic_id_t topic_id, TickType_t timeout)
{
    BaseType_t res = pdFALSE;

    while (timeout && topics_db[topic_id].is_locked)
    {
        vTaskDelay(pdMS_TO_TICKS(1));
        --timeout;
    }

    if (!topics_db[topic_id].is_locked)
    {
        topics_db[topic_id].is_locked = pdTRUE;
        res                           = pdTRUE;
    }
#if (CONFIG_MEMORY_MODEL_ENABLE_DEBUG == 1)
    else
    {
        ++lock_failure_count;
    }
#endif

    return res;
}

void IRAM_ATTR __attribute__((optimize("-O2")))
memory_model_unlock(topic_id_t topic_id)
{
    topics_db[topic_id].is_locked = pdFALSE;
}

size_t memory_model_get_lock_failures(void)
{
    return lock_failure_count;
}

// ----------------------------------------------------------------------
// FreeRTOS functions

#if (CONFIG_MEMORY_MODEL_ENABLE_DEBUG == 1)
static void IRAM_ATTR __attribute__((optimize("-O2")))
memory_model_report_timer(void* pvArg)
{
    (void)pvArg;

    ASYNC_PRINTF(CONFIG_MEMORY_MODEL_LOCK_FAIL_DBG_PRINTOUT, async_print_type_u32, "MM failures: %u\n", (uint32_t)memory_model_get_lock_failures());
}
#endif

// ----------------------------------------------------------------------
// Core functions

void init_memory_model(void)
{
    memory_model_validate();
    memory_model_init_rtos();

#if (CONFIG_MEMORY_MODEL_ENABLE_DEBUG == 1)
    xTimerStart(xMqttPublishTimer, MQTT_PUBLISH_TIMEOUT);
#endif
}