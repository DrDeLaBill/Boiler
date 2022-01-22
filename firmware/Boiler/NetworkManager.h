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

#include "RadioSensor.h"
#include "BoilerConstants.h"

class NetworkManager
{
  private:
    // ssid и pass точки доступа котла
    const char *soft_ap_ssid;        
    const char *soft_ap_password;
    
  public:
    static String current_ssid;
    static String current_pass;
    
    NetworkManager();
    void network_init();
    void connect_to_wifi(void);
    void server_init();
    void send_settings_to_server(void);
    void check_new_settings(BoilerConfiguration boiler_configuration);
    void set_wifi_settings(String ssid, String pass);
    String get_ssid();
    String get_pass();
    bool is_wifi_connected();
    uint8_t get_wifi_status();
};

#endif
