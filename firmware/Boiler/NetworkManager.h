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

#include "tprofile.h"
#include "clock_rtc.h"
#include "temp.h"
#include "ext_temp.h"

#define ARDUINOJSON_USE_LONG_LONG     1

#define WIFI_CONNECT_TIMEOUT          5000      // таймаут подключения к WiFi сети
#define WEB_REQUESTS_PERIOD           10000     // период отправки статуса на веб-сервер

// ssid и pass точки доступа котла
const char *soft_ap_ssid = "BoilerAP";        
const char *soft_ap_password = "12345678";

enum connect_status {
  DISCONNECTED = 0,
  CONNECTED,
  SETS_NOT_SENDED
};

class NetworkManager
{
  private:
    String current_ssid;
    String current_pass;
  public:
    NetworkManager();
    void network_init();
    void connect_to_wifi(void);
    void server_init();
    void send_settings_to_server(void);
    void check_new_settings(void);
    void set_wifi_settings(String ssid, String pass);
    String get_ssid();
    String get_pass();
};

#endif
