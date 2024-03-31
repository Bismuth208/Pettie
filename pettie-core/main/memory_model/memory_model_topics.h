#ifndef _MEMORY_MODEL_TOPICS_H
#define _MEMORY_MODEL_TOPICS_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif

// ----------------------------------------------------------------------
// Definitions, type & enum declaration

/// Maximum characters for topics, should be enough...
#define TOPICS_LEN_MAX (128u)

/// Maximum characters for data in topics, should be enough...
#define TOPICS_DATA_LEN_MAX (16u)


/// To test MQTT bandwidth and possible limits of the MCU
/// Use Python script from electron-mqtt-logger.
#define TOPIC_ENABLE_TEST_MODE 1

    // -----------------------------------------------
    /// List of all supported MQTT topics and internal variables
    typedef enum
    {
        // ----------------
        TOPIC_ID_SERVO_CAL_D0,
        TOPIC_ID_SERVO_CAL_D1,
        TOPIC_ID_SERVO_CAL_D2,
        // TOPIC_ID_SERVO_CAL_D3,
        TOPIC_ID_SERVO_CAL_D4,
        TOPIC_ID_SERVO_CAL_D5,
        TOPIC_ID_SERVO_CAL_D6,
        TOPIC_ID_SERVO_CAL_D7,
        TOPIC_ID_SERVO_CAL_D8,
        TOPIC_ID_SERVO_CAL_UPDATE,
        TOPIC_ID_SERVO_CAL_SAVE,

        // ----------------
        // TOPIC_ID_DISTANCE,

        // ----------------
        TOPIC_ID_RSSI_VAL,

        // ----------------
        TOPIC_ID_MAX
    } topic_id_t;


    // -----------------------------------------------
    /// Topic status
    typedef struct
    {
        /// Flasg to tell MQTT publish timer to send value
        bool do_publish;
        /// Flasg to tell MQTT what we publish this Topic
        bool need_publish;
        /// Flasg to tell MQTT what we will listen this Topic
        bool need_sub;
        ///
        uint64_t update_timestamp;
        /// Time between sending data
        uint64_t pub_timeout;
        ///
        uint64_t pub_timestamp;
    } topic_status_t;

    /// Topic status
    typedef struct
    {
        /// Array with Topic branch
        const char text[TOPICS_LEN_MAX];
        /// Array with Topic massage
        char value[TOPICS_DATA_LEN_MAX];
        /// Calculated length of the text Topic string (to reduce strnlen calculations)
        size_t text_len;
        /// Fixed maximum length of the value Topic string (to fix UI and System instability)
        const size_t value_len;
        /// Calculated Hash from the Topic Text.
        /// Needed for faster parsing of incoming MQTT messages
        uint32_t hash;
    } topic_data_t;

    /// Topic and/or Global variable descriptor
    typedef struct
    {
        /// Pointer to the Topic data structure with all necessary stuff
        topic_data_t data;
        ///
        topic_status_t status;
    } topic_t;

    /// Topic database registry
    typedef struct
    {
        /// Self Id must match to the index of topic_id_t in @ref topics array!
        const topic_id_t self_id;
        /// Pointer to the Topic data structure with all necessary stuff
        topic_t* topic;
        ///
        volatile bool is_locked;
    } topic_db_t;

    // ----------------------------------------------------------------------
    // Variables

    extern topic_db_t topics_db[];

#ifdef __cplusplus
}
#endif

#endif // _MEMORY_MODEL_TOPICS_H