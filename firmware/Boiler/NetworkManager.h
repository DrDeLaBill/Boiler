/*
 * 
 * настройки подключения к сети
 * 
 */
#ifndef _NETWORK_H_
#define _NETWORK_H_

#include <Arduino.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <Update.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>
#include "AsyncJson.h"
#include "ArduinoJson.h"

#include "BoilerConstants.h"
#include "RadioSensor.h"

class NetworkManager
{
  public:
    // ssid и pass точки доступа котла
    static const char *soft_ap_ssid;        
    static const char *soft_ap_password;
    
    static String current_ssid;
    static String current_pass;
    
    NetworkManager();
    static void network_init();
    static void connect_to_wifi(void);
    static void server_init();
    static void send_settings_to_server(void);
    static void check_new_settings(BoilerConfiguration boiler_configuration);
    static void set_wifi_settings(String ssid, String pass);
    static String get_ssid();
    static String get_pass();
    static bool is_wifi_connected();
    static uint8_t get_wifi_status();
};

#endif
