#ifndef _MQTT_CONFIG_H
#define _MQTT_CONFIG_H

#ifdef __cplusplus
extern "C"
{
#endif


// ----------------------------------------------------------------------
// Definitions, type & enum declaration

/// An a MQTT server address
#define MQTT_SERVER_ADDR "mqtt://YOUR_MQTT_IP" 

/// Port number for the MQTT broker
#define MQTT_PORT 1883

/// Amount of MQTT server to which device will try co connect
#define MQTT_MAX_SERVERS (1)

/// How often device will publish Topics
/// @note Only one Topic is sent per timer event!
/// @note One round will take (MQTT_PUBLISH_TIMEOUT * @ref TOPIC_ID_MAX) milliseconds
/// value is in ms
#define MQTT_PUBLISH_TIMEOUT (10)
/// How often device will try to reconnect to the MQTT broker
/// value is in ms
#define MQTT_RECONNECT_TIMEOUT (3000)
/// How often device will try to reconnect to the MQTT broker
/// This happens if established connection failed,
/// so we will try re-establish as fast as possible
/// value is in ms
#define MQTT_RECONNECT_FAIL_TIMEOUT (500)

/// General flag. Controls if device will send any MQTT topics
/// 1 - Publish is enabled
/// 0 - Publish is disabled
#define MQTT_ENABLE_PUBLISH 1



#ifdef __cplusplus
}
#endif

#endif // _MQTT_CONFIG_H
