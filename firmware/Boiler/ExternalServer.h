#ifndef _EXTERNAL_SERVER_H_
#define _EXTERNAL_SERVER_H_
#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "BoilerConstants.h"

class ExternalServer
{
  private:
    HTTPClient *http;
    const char* HEADER_TYPE = "Content-Type";
    const char* JSON_HEADER = "application/json";
    const char* WebServerAddr = "http://192.168.10.199";
    const char* boiler_id = "abcdabcd12";
  public:
    static uint32_t last_time_http;
    static bool got_new_wifi_settings;
    static bool connected_to_server;
    
    ExternalServer();
    void send_settings_to_server(uint8_t session_boiler_mode,
                                 uint8_t target_temp,
                                 BoilerConfiguration boiler_configuration);
    uint8_t get_connected_to_server();
    bool is_new_wifi_settings();
    void set_new_wifi_settings_flag(bool new_flag);
    void set_connected_to_server(bool value);
    void start_http(String url_to_serer);
    String get_web_server_address();
    void http_send_json(String json_string);
    void close_http_session();
    int http_get();
    String http_get_string();
    String get_http_error(int response_code);
    uint8_t get_new_wifi_settings();
};

#endif
