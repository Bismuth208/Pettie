#include "mqtt_func.h"

#include "common/common.h"
#include "common/hash_functions.h"
#include "common/pins.h"
#include "common/settings.h"
#include "mqtt_config.h"

//
#include "debug_tools.h"
//
#include <mqtt_client.h>
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
#include <esp_event.h>
#include <esp_log.h>
#include <esp_timer.h>
//
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

// ----------------------------------------------------------------------
// Definitions, type & enum declaration

typedef struct
{
    char msg[TOPICS_LEN_MAX];
    char value[TOPICS_DATA_LEN_MAX];
    size_t msg_len;
    size_t value_len;
} mqtt_queue_t;

// ----------------------------------------------------------------------
// FreeRTOS Variables

#define STACK_WORDS_SIZE_FOR_TASK_MQTTT (2048)
#define PRIORITY_LEVEL_FOR_TASK_MQTTT   (2)
#define PINNED_CORE_FOR_TASK_MQTTT      (1)
const char* assigned_name_for_task_mqtt = "mqtt_usr";
TaskHandle_t xMQTTTaskHandler           = NULL;
StaticTask_t xMQTTTTaskControlBlock;
StackType_t xMQTTTStack[STACK_WORDS_SIZE_FOR_TASK_MQTTT];

#if 0
// in ms
const char* assigned_name_for_mqtt_pub = "mqtt_pub";
TimerHandle_t xMqttPublishTimer        = NULL;
StaticTimer_t xMqttPublishTimerControlBlock;
#endif

//
#define MQTT_QUEUE_SIZE (8UL)
QueueHandle_t xMqttQueueHandler = NULL;
StaticQueue_t xMqttControlBlock;
size_t xMqttStorage[MQTT_QUEUE_SIZE];

// ----------------------------------------------------------------------
// Variables

const char* mqtt_servers[] = {
    MQTT_SERVER_ADDR, // Home MQTT
};
_Static_assert((sizeof(mqtt_servers) / sizeof(mqtt_servers[0])) == (MQTT_MAX_SERVERS), "MQTT servers count mismatch!");

// clang-format off
esp_mqtt_client_config_t mqtt_cfg = {
  .broker.address.uri = MQTT_SERVER_ADDR,
  .credentials.client_id = "Pettie",
  .network.timeout_ms = MQTT_RECONNECT_TIMEOUT * 2,
  .network.reconnect_timeout_ms = MQTT_RECONNECT_TIMEOUT,
  .task.priority = PRIORITY_LEVEL_FOR_TASK_MQTTT
};
// clang-format on


static esp_mqtt_client_handle_t mqtt_client;
static BaseType_t mqtt_is_connected = pdFALSE;
static size_t mqtt_current_srv      = 0u;

// static size_t last_mqtt_msg_offset = 0u;

#define MQTT_RING_BUFFER_SIZE (8U) // must be power of 2!
#define MQTT_RING_BUFFER_MASK (MQTT_RING_BUFFER_SIZE - 1)

mqtt_queue_t mqtt_data_ring_buf[MQTT_RING_BUFFER_SIZE];
size_t mqtt_ring_buf_head = 1ul;
size_t mqtt_ring_buf_tail = 0ul;

#if (MQTT_QUEUE_SIZE > MQTT_RING_BUFFER_SIZE)
#    error "MQTT_QUEUE_SIZE cannot be more than MQTT_RING_BUFFER_SIZE !"
#endif

// ----------------------------------------------------------------------
// Static functions declaration

static void init_mqtt_rtos(void);

static void mqtt_task(void* pvArg);

#if 0
static void mqtt_publish_timer(void* pvArg);
#endif


/**
 * @brief Subscribes to all MQTT topics listed in @ref topics
 *
 * @note It will subscribe on if .need_sub = true
 */
static void mqtt_subscribe(void);

/**
 * @brief Called when new MQTT topic received
 *
 * @param [in] topic Text string with topic text
 * @param [in] message Text string with topic playload
 * @param [in] topic_length Length of the @ref topic
 * @param [in] msg_length Length of the @ref message
 *
 * @note This function is called from MqttClient.loop();
 *       it locks Mutex before entering here.
 */
static void mqtt_recieve_callback(const char* topic, const char* message, size_t topic_length, size_t msg_length);

// ----------------------------------------------------------------------
// Static functions

static void mqtt_subscribe(void)
{
    topic_db_t* px_topic = NULL;

    for (size_t i = 0u; i < TOPIC_ID_MAX; i++)
    {
        px_topic = &topics_db[i];

        if ((px_topic->self_id != TOPIC_ID_MAX) && (px_topic->topic != NULL))
        {
            if (px_topic->topic->status.need_sub)
            {
                esp_mqtt_client_subscribe(mqtt_client, px_topic->topic->data.text, 0);
            }
        }
    }
}

static void IRAM_ATTR __attribute__((optimize("-O2")))
mqtt_recieve_callback(const char* topic, const char* message, size_t topic_length, size_t msg_length)
{
    // PROFILE_POINT(CONFIG_MQTT_HASH_DBG_PROFILER, profile_point_start);
    uint32_t new_topic_hash = ul_get_hash(topic, strnlen(topic, topic_length));
    // PROFILE_POINT(CONFIG_MQTT_HASH_DBG_PROFILER, profile_point_end);

    topic_db_t* px_topic = NULL;

    for (size_t i = 0u; i < TOPIC_ID_MAX; i++)
    {
        px_topic = &topics_db[i];

        if (new_topic_hash == px_topic->topic->data.hash)
        {
            // Try to lock MM
            if (memory_model_lock(px_topic->self_id, portMAX_DELAY))
            {
                if (px_topic->topic->data.value_len == 0)
                {
                    msg_length = fmin(msg_length, TOPICS_DATA_LEN_MAX - 1u);
                }
                else
                {
                    msg_length = fmin(msg_length, px_topic->topic->data.value_len - 1u);
                }

                strncpy(&px_topic->topic->data.value[0], &message[0], msg_length);
                px_topic->topic->data.value[msg_length] = '\0';

                memory_model_update(px_topic->topic, pdFALSE);
                memory_model_unlock(px_topic->self_id);
            }
            break;
        }
    }
}

/**
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void IRAM_ATTR mqtt_event_handler(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data)
{
    (void)handler_args;
    (void)base;

    esp_mqtt_event_handle_t event   = event_data;
    esp_mqtt_client_handle_t client = event->client;

    switch ((esp_mqtt_event_id_t)event_id)
    {
        case MQTT_EVENT_CONNECTED: {
            if (mqtt_is_connected == pdFALSE)
            {
                mqtt_is_connected = pdTRUE;
                mqtt_subscribe();

                ASYNC_PRINTF(CONFIG_MQTT_CONN_STATE_DBG_PRINTOUT, async_print_type_str, "MQTT oMQ", 0);
            }
            break;
        }

        case MQTT_EVENT_DISCONNECTED: {
            if (mqtt_is_connected == pdTRUE)
            {
                mqtt_is_connected = pdFALSE;

                ASYNC_PRINTF(CONFIG_MQTT_CONN_STATE_DBG_PRINTOUT, async_print_type_str, "MQTT nMQ", 0);
            }
            break;
        }

        case MQTT_EVENT_DATA: {
            if (!event->dup)
            {
                mqtt_queue_t* mqtt_data;

                // check if there any free space in ring buffer
                if (((mqtt_ring_buf_head + 1) & MQTT_RING_BUFFER_MASK) != mqtt_ring_buf_tail)
                {
                    // PROFILE_POINT(CONFIG_MQTT_DATA_EVENT_DBG_PROFILER, profile_point_start);

                    mqtt_ring_buf_head = (mqtt_ring_buf_head + 1) & MQTT_RING_BUFFER_MASK;

                    mqtt_data            = &mqtt_data_ring_buf[mqtt_ring_buf_head];
                    mqtt_data->msg_len   = fmin(event->topic_len, TOPICS_LEN_MAX);
                    mqtt_data->value_len = fmin(event->data_len, TOPICS_DATA_LEN_MAX);

                    memcpy(&mqtt_data->msg[0], &event->topic[0], mqtt_data->msg_len);
                    memcpy(&mqtt_data->value[0], &event->data[0], mqtt_data->value_len);

                    // If no free space don't block MQTT low level task!
                    xQueueSend(xMqttQueueHandler, &mqtt_ring_buf_head, 0);

                    // PROFILE_POINT(CONFIG_MQTT_DATA_EVENT_DBG_PROFILER, profile_point_end);
                }
            }
            break;
        }

        case MQTT_EVENT_ERROR: {
            // If connection failed try next Brocker
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
            {
                ++mqtt_current_srv;
                if (mqtt_current_srv >= MQTT_MAX_SERVERS)
                {
                    mqtt_current_srv = 0u;
                }

                esp_mqtt_client_set_uri(client, mqtt_servers[mqtt_current_srv]);
            }
            break;
        }

        default:
            break;
    }
}

static void init_mqtt_rtos(void)
{
#if 0
    xMqttPublishTimer = xTimerCreateStatic(assigned_name_for_mqtt_pub,
                                           pdMS_TO_TICKS(MQTT_PUBLISH_TIMEOUT),
                                           pdTRUE,
                                           NULL,
                                           (TimerCallbackFunction_t)(mqtt_publish_timer),
                                           &xMqttPublishTimerControlBlock);
    assert(xMqttPublishTimer);
#endif

    xMqttQueueHandler = xQueueCreateStatic(MQTT_QUEUE_SIZE, sizeof(size_t), (uint8_t*)&xMqttStorage[0], &xMqttControlBlock);
    assert(xMqttQueueHandler);

    xMQTTTaskHandler = xTaskCreateStaticPinnedToCore((TaskFunction_t)(mqtt_task),
                                                     assigned_name_for_task_mqtt,
                                                     STACK_WORDS_SIZE_FOR_TASK_MQTTT,
                                                     NULL,
                                                     PRIORITY_LEVEL_FOR_TASK_MQTTT,
                                                     xMQTTTStack,
                                                     &xMQTTTTaskControlBlock,
                                                     (BaseType_t)PINNED_CORE_FOR_TASK_MQTTT);
    assert(xMQTTTaskHandler);
}

// ----------------------------------------------------------------------
// Accessors functions


void mqtt_publish(const char *topic, const char *data)
{
  // Send to the MQTT queue to send in it's own task
  // All Tx is done with QOS1
  // esp_mqtt_client_enqueue(mqtt_client, topic, data, 0, 0, 0, 0);

  // Direct publish
  esp_mqtt_client_publish(mqtt_client, topic, data, 0, 0, 0);
}


// ----------------------------------------------------------------------
// FreeRTOS functions

#if 0
/**
 * @brief MQTT Publish data timer
 *
 * @param [in] *pvArg pointer to provided arguments
 *
 * @note Must be called every 100ms
 */
static void IRAM_ATTR __attribute__((optimize("-O2")))
mqtt_publish_timer(void* pvArg)
{
    (void)pvArg;

    if (mqtt_is_connected == pdFALSE)
    {
        return;
    }

    topic_db_t* px_topic = &topics_db[last_mqtt_msg_offset];

    ++last_mqtt_msg_offset;
    if (last_mqtt_msg_offset >= TOPIC_ID_MAX)
    {
        last_mqtt_msg_offset = 0u;
    }

    // We cannot block Timer for long periods
    // Or other FreeFRTOS timers will break.
    if (memory_model_lock(px_topic->self_id, 1) == pdFALSE)
    {
        return;
    }

    if (px_topic->topic->status.need_publish && px_topic->topic->status.do_publish)
    {
        if ((system_get_us_time() - px_topic->topic->status.pub_timestamp) >= px_topic->topic->status.pub_timeout)
        {
#if MQTT_ENABLE_PUBLISH
            // PROFILE_POINT(CONFIG_MQTT_DATA_PUB_DBG_PROFILER, profile_point_start);

            // Send to the MQTT queue to send in it's own task
            // All Tx is done with QOS1
            esp_mqtt_client_enqueue(mqtt_client, px_topic->topic->data.text, px_topic->topic->data.value, 0, 1, 0, 0);

            // PROFILE_POINT(CONFIG_MQTT_DATA_PUB_DBG_PROFILER, profile_point_end);
#endif
            px_topic->topic->status.pub_timestamp = system_get_us_time();
            px_topic->topic->status.do_publish    = false;
        }
    }

    memory_model_unlock(px_topic->self_id);
}
#endif

static void IRAM_ATTR __attribute__((optimize("-O2")))
mqtt_task(void* pvArg)
{
    (void)pvArg;

    task_sync_get_bits(TASK_SYNC_EVENT_BIT_MQTT);

    mqtt_queue_t* mqtt_data;
    size_t mqtt_data_index = 0ul;

    for (;;)
    {
        if (xQueueReceive(xMqttQueueHandler, &mqtt_data_index, portMAX_DELAY))
        {
            mqtt_data = &mqtt_data_ring_buf[mqtt_data_index];
            mqtt_recieve_callback(mqtt_data->msg, mqtt_data->value, mqtt_data->msg_len, mqtt_data->value_len);

            mqtt_ring_buf_tail = (mqtt_ring_buf_tail + 1) & MQTT_RING_BUFFER_MASK;
        }
    }
}

// ----------------------------------------------------------------------
// Core functions

void init_mqtt(void)
{
    init_mqtt_rtos();

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this case mqtt_event_handler */
    esp_mqtt_client_register_event(mqtt_client, MQTT_EVENT_CONNECTED, mqtt_event_handler, NULL);
    esp_mqtt_client_register_event(mqtt_client, MQTT_EVENT_DISCONNECTED, mqtt_event_handler, NULL);
    esp_mqtt_client_register_event(mqtt_client, MQTT_EVENT_DATA, mqtt_event_handler, NULL);
    esp_mqtt_client_register_event(mqtt_client, MQTT_EVENT_ERROR, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);

#if 0
    xTimerStart(xMqttPublishTimer, MQTT_PUBLISH_TIMEOUT);
#endif
}
