#ifndef _EXTERNAL_SERVER_H_
#define _EXTERNAL_SERVER_H_

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "BoilerConstants.h"
#include "DisplayManager.h"
#include "NetworkManager.h"
#include "BoilerProfile.h"
#include "InternalServer.h"

class ExternalServer
{
  public:
    static const char* HEADER_TYPE;
    static const char* JSON_HEADER;
    static const char* WebServerAddr;
    static const char* boiler_id;
  
    static HTTPClient http;
    static uint32_t last_time_http;
    static bool got_new_wifi_settings;
    static bool connected_to_server;
    
    static void check_settings();
    static void profile_settings_init(String url);
    static uint8_t get_new_wifi_settings();
    static void set_new_wifi_settings_flag(bool new_flag);
    static void send_settings_to_server();
    static String get_web_server_address();
    static void serial_error_report(String target_url, int response_code);
    static void close_http_session();
    static void set_connected_to_server(bool value);
    static void start_http(String url_to_serer);
    static String http_get_string();
    static int http_get();
    static String get_http_error(int response_code);
    static uint8_t get_connected_to_server();
    static int http_send_json(String json_string);
};

#endif
