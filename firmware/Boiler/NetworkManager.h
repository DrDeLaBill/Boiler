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
#include "BoilerProfile.h"

class NetworkManager
{
  public:
    // ssid и pass точки доступа котла
    static const String soft_ap_ssid;
    static const String soft_ap_password;

    NetworkManager();
    static String current_ssid;
    static String current_pass;
    static void check_new_settings();
    static String get_ssid();
    static String get_pass();
    static void connect_to_wifi(uint16_t connect_timeout = WIFI_CONNECT_TIMEOUT);
    static void send_settings_to_server();
    static uint8_t get_wifi_status();
    static bool is_wifi_connected();
    static void set_wifi_settings(String ssid, String pass);
};

#endif
