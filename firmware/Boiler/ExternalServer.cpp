#include "ExternalServer.h"

uint32_t ExternalServer::last_time_http = 0;
bool ExternalServer::got_new_wifi_settings = false;
bool ExternalServer::connected_to_server = DISCONNECTED;

ExternalServer::ExternalServer() {
  this->http = new HTTPClient();
}

void ExternalServer::send_settings_to_server(uint8_t session_boiler_mode,
                                             uint8_t target_temp,
                                             BoilerConfiguration boiler_configuration) {
    // Настройки отправляем при включении и при каждом их изменении.
    
    String path_to_server = String(this->WebServerAddr)
                          + "/boiler/"
                          + String(this->boiler_id);
    
    DynamicJsonDocument doc(150);
    int response_code;
    
    String url_to_server = path_to_server + "/settings";
    this->start_http(url_to_server);
    this->http->setConnectTimeout(100);
    this->http->addHeader(HEADER_TYPE, JSON_HEADER);

    // текущий режим работы и выставленная температура
    if (session_boiler_mode == MODE_AIR) {
        doc["mode"] = String(S_SETPOINT);
    } else if (session_boiler_mode == MODE_WATER) {
        doc["mode"] = String(S_SETPOINTWATER);
    } else if (session_boiler_mode == MODE_PROFILE) {
        doc["mode"] = String(S_PROFILE);
    }
    doc["target_temp"] = target_temp;
    String send_json = "";
    serializeJson(doc, send_json);
    response_code = this->http->PUT(send_json);
    if (response_code > 0){
      ExternalServer::connected_to_server = CONNECTED;
    } else {
      ExternalServer::connected_to_server = SETS_NOT_SENDED;
      Serial.print(F("path: "));
      Serial.println(url_to_server);
      Serial.print(F("response_code: "));
      Serial.println(this->get_http_error(response_code));
    }
    this->close_http_session();
    doc.clear();

    // отправляем настройки всех профилей
    url_to_server = path_to_server + "/profile/";

    for (uint8_t j = 0; j < NUM_PRESETS; j++){
      String url_profile = url_to_server + DisplayManager::presets[j];
      this->start_http(url_profile);
      this->http->setConnectTimeout(100);
      this->http->addHeader(HEADER_TYPE, JSON_HEADER);
      for (uint8_t i = 0; i < NUM_PERIODS; i++){
        String p_period = "h";
        p_period += String(i);
        doc[p_period] = boiler_configuration.presets[j][i];
      }
      send_json = "";
      serializeJson(doc, send_json);
      response_code = this->http->PUT(send_json);
      if (response_code > 0){
        ExternalServer::connected_to_server = CONNECTED;
      } else {
        ExternalServer::connected_to_server = SETS_NOT_SENDED;
        Serial.print(F("path: "));
        Serial.println(url_to_server);
        Serial.print(F("response_code: "));
        Serial.println(this->get_http_error(response_code));
      }
      this->close_http_session();
    }
    doc.clear();

    // отправляем профиль на неделю
    url_to_server = path_to_server + "/week";
    this->start_http(url_to_server);
    this->http->setConnectTimeout(100);
    this->http->addHeader(HEADER_TYPE, JSON_HEADER);
    for (uint8_t i = 0; i < NUM_DAYS; i++){
      String p_day = "d";
      p_day += String(i);
      doc[p_day] = DisplayManager::presets[boiler_configuration.profile[i]];
    }
    send_json = "";
    serializeJson(doc, send_json);
    response_code = this->http->PUT(send_json);
    if (response_code > 0){
      ExternalServer::connected_to_server = CONNECTED;
    } else {
      ExternalServer::connected_to_server = SETS_NOT_SENDED;
      Serial.print(F("path: "));
      Serial.println(url_to_server);
      Serial.print(F("response_code: "));
      Serial.println(this->get_http_error(response_code));
    }
    this->close_http_session();
    doc.clear();
}

void ExternalServer::start_http(String url_to_server) {
  this->http->begin(url_to_server);
  this->http->addHeader(HEADER_TYPE, JSON_HEADER);
}

uint8_t ExternalServer::get_connected_to_server() {
  return ExternalServer::connected_to_server;
}

uint8_t ExternalServer::get_new_wifi_settings() {
  return ExternalServer::got_new_wifi_settings;
}

void ExternalServer::set_new_wifi_settings_flag(bool new_flag) {
  ExternalServer::got_new_wifi_settings = new_flag;
}

String ExternalServer::get_web_server_address() {
  return this->WebServerAddr;
}

void ExternalServer::http_send_json(String json_string) {
  this->http->PUT(json_string);
}

void ExternalServer::close_http_session() {
  this->http->end();
}

int ExternalServer::http_get() {
  return this->http->GET();
}

String ExternalServer::http_get_string() {
  return this->http->getString();
}

String ExternalServer::get_http_error(int response_code) {
  return this->http->errorToString(response_code);
}

void ExternalServer::set_connected_to_server(bool value) {
  ExternalServer::connected_to_server = value;
}
