#ifndef _MEMORY_MODEL_H
#define _MEMORY_MODEL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "memory_model_topics.h"

//
#include <sdkconfig.h>
//
#include <esp_attr.h>
#include <freertos/FreeRTOS.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

    // ----------------------------------------------------------------------
    // Definitions, type & enum declaration


    // -----------------------------------------------
    /// Control Float formatting precision
    typedef enum
    {
        /// format float as ?.00
        STR_2_FLOAT_DOT_2 = 2,
        /// format float as ?.00000
        STR_2_FLOAT_DOT_5 = 5,
        /// format float as ?.000000000
        STR_2_FLOAT_DOT_7 = 7,
    } str_2_float_dot_t;

    // ----------------------------------------------------------------------
    // Accessors functions

    /**
     * @brief Update all status flags relevant to Topic
     *
     * @param [in] px_topic pointer to the MQTT topic id in @ref topics_db
     * @param [in] xPublish do the publish pdTRUE if need to send via MQTT
     */
    void memory_model_update(topic_t* px_topic, BaseType_t xPublish);

    /**
     * @brief Converts float values to string
     *
     * @param [in] topic_id MQTT topic id in @ref topics
     * @param [in] value float value to convert
     * @param [in] precision amount of digits after decemical point
     *
     * @note This function locks Mutex indefinitely!
     */
    void memory_model_set_float(topic_id_t topic_id, float value, size_t precision);

    /**
     * @brief Converts boolean values to string
     *
     * @param [in] topic_id MQTT topic id in @ref topics
     * @param [in] value boolean value to convert
     *
     * @note This function locks Mutex indefinitely!
     */
    void memory_model_set_bool(topic_id_t topic_id, bool value);

    /**
     * @brief Converts u32 values to string
     *
     * @param [in] topic_id MQTT topic id in @ref topics
     * @param [in] value u32 value to convert
     *
     * @note This function locks Mutex indefinitely!
     */
    void memory_model_set_u32(topic_id_t topic_id, uint32_t value);

    /**
     * @brief Converts i32 values to string
     *
     * @param [in] topic_id MQTT topic id in @ref topics
     * @param [in] value i32 value to convert
     *
     * @note This function locks Mutex indefinitely!
     */
    void memory_model_set_i32(topic_id_t topic_id, int32_t value);

    /**
     * @brief Copy Str values to String
     *
     * @param [in] topic_id MQTT topic id in @ref topics
     * @param [in] value str to copy
     *
     * @note This function locks Mutex indefinitely!
     */
    void memory_model_set_str(topic_id_t topic_id, const char* value);


    bool memory_model_get_bool(topic_id_t topic_id, TickType_t timeout);
    int32_t memory_model_get_i32(topic_id_t topic_id, TickType_t timeout);
    float memory_model_get_float(topic_id_t topic_id, TickType_t timeout);

    bool memory_model_is_same(topic_id_t topic_id, TickType_t timeout, const char* val);


    /**
     * @brief Lock access to the global MQTT topics
     * 
     * @param [in] timeout value in Ticks
     * @return pdTRUE if mutex is locked
     *
     * @note @ref topics in global vars is used to multi-modules access
     */
    BaseType_t memory_model_lock(topic_id_t topic_id, TickType_t timeout);

    /**
     * @brief Unlock access to the global MQTT topics
     *
     * @note @ref topics in global vars is used to multi-modules access
     */
    void memory_model_unlock(topic_id_t topic_id);

    /**
     * @brief Debug stats
     * 
     * @note Requires to enable @ref MEMORY_MODEL_ENABLE_DEBUG
     */
    size_t memory_model_get_lock_failures(void);

    // ----------------------------------------------------------------------
    // Core functions

    void init_memory_model(void);


#ifdef __cplusplus
}
#endif

#endif // _MEMORY_MODEL_H