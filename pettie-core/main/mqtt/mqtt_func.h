#ifndef _MQTT_FUNC_H
#define _MQTT_FUNC_H

#ifdef __cplusplus
extern "C"
{
#endif

// ----------------------------------------------------------------------
#include "memory_model/memory_model.h"

//
#include <sdkconfig.h>
//
#include <freertos/FreeRTOS.h>
#include <stddef.h>
#include <stdint.h>


// ----------------------------------------------------------------------
// Definitions, type & enum declaration


    // ----------------------------------------------------------------------
    // Variables


    // ----------------------------------------------------------------------
    // Accessors functions

    void mqtt_publish(const char *topic, const char *data);


    // ----------------------------------------------------------------------
    // Core functions

    /**
     * @brief Initialize MQTT client
     */
    void init_mqtt(void);

#ifdef __cplusplus
}
#endif

#endif // _MQTT_FUNC_H
