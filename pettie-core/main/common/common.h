#ifndef _COMMON_H
#define _COMMON_H

#ifdef __cplusplus
extern "C"
{
#endif

// ----------------------------------------------------------------------
#include <stdint.h>

    // ----------------------------------------------------------------------
    // Definitions, type & enum declaration

#define TASK_SYNC_EVENT_BIT_UI_CORE       (1 << 0)
#define TASK_SYNC_EVENT_BIT_INPUT_HANDLER (1 << 1)
#define TASK_SYNC_EVENT_BIT_INPUT_ADS     (1 << 3)
#define TASK_SYNC_EVENT_BIT_GPS           (1 << 4)
#define TASK_SYNC_EVENT_BIT_MQTT          (1 << 5)

#define TASK_SYNC_EVENT_BIT_ALL                                                                                                                                                    \
    (TASK_SYNC_EVENT_BIT_UI_CORE | TASK_SYNC_EVENT_BIT_INPUT_ADS | TASK_SYNC_EVENT_BIT_INPUT_HANDLER | TASK_SYNC_EVENT_BIT_GPS | TASK_SYNC_EVENT_BIT_MQTT)


#define VERSION_STR "v0.4"
#define OTA_STR     "OTA"


// ----------------------------------------------------------------------

    // ----------------------------------------------------------------------

    /**
     * @brief Just a linear range converter
     * 
     * @param
     * @param
     * @param
     * @param
     * @param
     * 
     * @retval Convereted value
     */
    int32_t ul_map_val(const int32_t x, int32_t imin, int32_t imax, int32_t omin, int32_t omax);


    // void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax);

    void task_sync_set_bits(uint32_t ulBits);
    void task_sync_get_bits(uint32_t ulBits);

    int64_t system_get_us_time(void);

#ifdef __cplusplus
}
#endif

#endif // _COMMON_H
