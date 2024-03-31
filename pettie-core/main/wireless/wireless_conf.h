/**
 * @file wireless_stuff_conf.h
 * 
 * Contains definitions for WiFi
 */

#ifndef _WIRELESS_STUFF_CONF_H
#define _WIRELESS_STUFF_CONF_H

#ifdef __cplusplus
extern "C"
{
#endif


// Settings of the Wi-Fi
// This will be used to connect to MQTT broker !
#define WIFI_SSID_NAME      "YOUR_SSID_NAME"
#define WIFI_SSID_PASSWORD  "YOUR_SSID_PASSWORD"

// By default device will use as much as possible power for Transmitting
// Declare value in percentage with range [WIFI_MIN_TX_POWER_PERCENTAGE : WIFI_MAX_TX_POWER_PERCENTAGE]
#define DEFAULT_WIFI_TX_POWER (100)

#define WIFI_MIN_TX_POWER_PERCENTAGE (1)
#define WIFI_MAX_TX_POWER_PERCENTAGE (100)

#if (DEFAULT_WIFI_TX_POWER < WIFI_MIN_TX_POWER_PERCENTAGE)
#    error "WiFi Tx power 1 is too low, see WIFI_MIN_TX_POWER_PERCENTAGE"
#endif

#if (DEFAULT_WIFI_TX_POWER > WIFI_MAX_TX_POWER_PERCENTAGE)
#    error "WiFi Tx power 1 is too high, see WIFI_MAX_TX_POWER_PERCENTAGE"
#endif

// Range [8, 84] corresponds to 2dBm - 20dBm and is taken from the documentation.
#define WIFI_MIN_TX_POWER (8)
#define WIFI_MAX_TX_POWER (84)

// GB should be fine ? Right?
//  “AT”,”AU”,”BE”,”BG”,”BR”,
//  “CA”,”CH”,”CN”,”CY”,”CZ”,”DE”,”DK”,”EE”,”ES”,”FI”,”FR”,”GB”,”GR”,”HK”,”HR”,”HU”,
//  “IE”,”IN”,”IS”,”IT”,”JP”,”KR”,”LI”,”LT”,”LU”,”LV”,”MT”,”MX”,”NL”,”NO”,”NZ”,”PL”,”PT”,
//  “RO”,”SE”,”SI”,”SK”,”TW”,”US”
#define DEFAULT_WIFI_COUNTRY_CODE ("GB")


#ifdef __cplusplus
}
#endif
#endif /* _WIRELESS_STUFF_CONF_H */