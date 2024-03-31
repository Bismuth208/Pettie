#ifndef _WIRELESS_H
#define _WIRELESS_H


// ----------------------------------------------------------------------
// Definitions, type & enum declaration

/// Amount of Wi-Fi networks to which device will try co connect
/// Place networks into @ref wifi_networks array
#define MAX_WIFI_NETWORKS (1)

/// How oftten to snif WiFi transmitter and update GUI
#define WIFI_RSSI_UPDATE_TIMEOUT (250)

// -----------------------
typedef struct
{
    const char* ssid;
    const char* password;
} wifi_network_t;

// ----------------------------------------------------------------------
// Core functions

void init_wifi();

#endif // _WIRELESS_H
