#include "ExternalServer.h"

uint32_t ExternalServer::last_time_http = 0;
bool ExternalServer::got_new_wifi_settings = false;
bool ExternalServer::connected_to_server = DISCONNECTED;
HTTPClient ExternalServer::http;
const char* ExternalServer::HEADER_TYPE = "Content-Type";
const char* ExternalServer::JSON_HEADER = "application/json";
const char* ExternalServer::WebServerAddr = "http://192.168.10.199";
const char* ExternalServer::boiler_id = "abcdabcd12";

void ExternalServer::send_settings_to_server() {
    // Настройки отправляем при включении и при каждом их изменении.
    uint8_t session_boiler_mode = BoilerProfile::get_session_boiler_mode();
    uint8_t target_temp = BoilerProfile::get_target_temp();
    
    
    String path_to_server = String(ExternalServer::WebServerAddr)
                          + "/boiler/"
                          + String(ExternalServer::boiler_id);
    
    DynamicJsonDocument doc(150);
    int response_code;
    
    String url_to_server = path_to_server + "/settings";
    ExternalServer::profile_settings_init(url_to_server);
    ExternalServer::http.setConnectTimeout(100);
    ExternalServer::http.addHeader(ExternalServer::HEADER_TYPE, ExternalServer::JSON_HEADER);

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
    response_code = ExternalServer::http.PUT(send_json);
    if (response_code > 0){
      ExternalServer::connected_to_server = CONNECTED;
    } else {
      ExternalServer::connected_to_server = SETS_NOT_SENDED;
      Serial.print(F("path: "));
      Serial.println(url_to_server);
      Serial.print(F("response_code: "));
      Serial.println(ExternalServer::get_http_error(response_code));
    }
    ExternalServer::close_http_session();
    doc.clear();

    // отправляем настройки всех профилей
    url_to_server = path_to_server + "/profile/";

    for (uint8_t j = 0; j < NUM_PRESETS; j++){
      String url_profile = url_to_server + DisplayManager::presets[j];
      ExternalServer::profile_settings_init(url_profile);
      ExternalServer::http.setConnectTimeout(100);
      ExternalServer::http.addHeader(ExternalServer::HEADER_TYPE, ExternalServer::JSON_HEADER);
      for (uint8_t i = 0; i < NUM_PERIODS; i++){
        String p_period = "h";
        p_period += String(i);
        doc[p_period] = BoilerProfile::boiler_configuration.presets[j][i];
      }
      send_json = "";
      serializeJson(doc, send_json);
      response_code = ExternalServer::http.PUT(send_json);
      if (response_code > 0){
        ExternalServer::connected_to_server = CONNECTED;
      } else {
        ExternalServer::connected_to_server = SETS_NOT_SENDED;
        Serial.print(F("path: "));
        Serial.println(url_to_server);
        Serial.print(F("response_code: "));
        Serial.println(ExternalServer::get_http_error(response_code));
      }
      ExternalServer::close_http_session();
    }
    doc.clear();

    // отправляем профиль на неделю
    url_to_server = path_to_server + "/week";
    ExternalServer::profile_settings_init(url_to_server);
    ExternalServer::http.setConnectTimeout(100);
    ExternalServer::http.addHeader(ExternalServer::HEADER_TYPE, ExternalServer::JSON_HEADER);
    for (uint8_t i = 0; i < NUM_DAYS; i++){
      String p_day = "d";
      p_day += String(i);
      doc[p_day] = DisplayManager::presets[BoilerProfile::boiler_configuration.profile[i]];
    }
    send_json = "";
    serializeJson(doc, send_json);
    response_code = ExternalServer::http.PUT(send_json);
    if (response_code > 0){
      ExternalServer::connected_to_server = CONNECTED;
    } else {
      ExternalServer::connected_to_server = SETS_NOT_SENDED;
      Serial.print(F("path: "));
      Serial.println(url_to_server);
      Serial.print(F("response_code: "));
      Serial.println(ExternalServer::get_http_error(response_code));
    }
    ExternalServer::close_http_session();
    doc.clear();
}

//TODO: сократить повторяющиеся части кода в функции (также для profile_settings_init())
//TODO:  проверить, не перепутались ли функции при измении их на статические, к примеру при замене http_get_string() и http_get(), тк они очень похожи
void ExternalServer::check_settings() {
  if (ExternalServer::get_new_wifi_settings()) {
    NetworkManager::connect_to_wifi();
    ExternalServer::set_new_wifi_settings_flag(false);
  }
  // Проверяем подключение к интернету.
  if (NetworkManager::is_wifi_connected()){
    // отправляем статус и запрашиваем настройки раз в минуту.
    
    if (millis() - ExternalServer::last_time_http >= WEB_REQUESTS_PERIOD){
      ExternalServer::last_time_http = millis();
      int response_code;
      
      if (ExternalServer::get_new_wifi_settings() == SETS_NOT_SENDED){
        ExternalServer::send_settings_to_server();
      }
      
      String path_to_server = String(ExternalServer::get_web_server_address()) 
                            + "/boiler/" 
                            + String(BoilerProfile::get_boiler_id());
      DynamicJsonDocument doc(150);
      
      // отправляем текущий статус котла
      String url_to_server = path_to_server + "/status";
      ExternalServer::profile_settings_init(url_to_server);
      doc["temp"] = TemperatureSensor::get_current_temperature();
      doc["target_temp"] = BoilerProfile::get_target_temp();
      uint8_t num_preset = BoilerProfile::get_profile_for_week_day();
      doc["current_profile"] = InternalServer::get_preset(num_preset);
      if (BoilerProfile::is_mode_air())
        doc["current_mode"] = String(InternalServer::get_s_setpoint());
      else if (BoilerProfile::is_mode_profile()) 
        doc["current_mode"] = String(InternalServer::get_s_profile());
      else if (BoilerProfile::is_mode_water()) 
        doc["current_mode"] = String(InternalServer::get_s_setpointwater());
      String send_json = "";
      serializeJson(doc, send_json);
      ExternalServer::http_send_json(send_json);
      ExternalServer::close_http_session();
      doc.clear();
  
      // проверяем есть ли новые настройки
      url_to_server = path_to_server + "/changed_by_client";
      ExternalServer::profile_settings_init(url_to_server);
      response_code = ExternalServer::http_get();
      if (response_code > 0){
        ExternalServer::set_connected_to_server(true);
        deserializeJson(doc, ExternalServer::http_get_string());
        ExternalServer::close_http_session();
        // проверяем обновления по всем настройкам
        if (doc["settings"] == true){
          // есть изменения в режиме работы
          
          url_to_server = path_to_server + "/settings";
          ExternalServer::profile_settings_init(url_to_server);
          int response_code = ExternalServer::http_get();
            if (response_code > 0){
              DynamicJsonDocument sets(100);
              deserializeJson(sets,ExternalServer::http_get_string());
              ExternalServer::close_http_session();
              
              if (sets["mode"] == InternalServer::get_s_setpoint()){
                // работаем по воздуху
                BoilerProfile::set_boiler_mode(MODE_AIR);
              } else if (sets["mode"] == InternalServer::get_s_setpointwater()){
                // работаем по теплоносителю
                BoilerProfile::set_boiler_mode(MODE_WATER);
              } else if (sets["mode"] == InternalServer::get_s_profile()){
                // работаем по термопрофилю
                BoilerProfile::set_boiler_mode(MODE_PROFILE);
              }
              uint8_t need_temp = sets["target_temp"];
              if (!(need_temp >= WATER_TEMP_MIN && need_temp <= WATER_TEMP_MAX)) {
                need_temp = WATER_TEMP_MIN;
              }
              BoilerProfile::set_target_temp(need_temp);
            } else {
              ExternalServer::serial_error_report(url_to_server, response_code);
            }
        }
  
        if (doc["profile_weekdays"] == true){
          // есть изменения в профиле
          url_to_server = path_to_server + "/profile/weekdays";
          ExternalServer::profile_settings_init(url_to_server);
        }
  
        if (doc["profile_weekend"] == true){
          // есть изменения в профиле
          url_to_server = path_to_server + "/profile/weekend";
          ExternalServer::profile_settings_init(url_to_server);
        }
  
        if (doc["profile_custom"] == true){
          // есть изменения в профиле
          url_to_server = path_to_server + "/profile/custom";
          ExternalServer::profile_settings_init(url_to_server);
        }
  
        if (doc["profile_notfreeze"] == true){
          // есть изменения в профиле
          url_to_server = path_to_server + "/profile/notfreeze";
          ExternalServer::profile_settings_init(url_to_server);
        }
  
        if (doc["week"] == true){
          // есть изменения в графике на неделю
  
          url_to_server = path_to_server + "/week";
          ExternalServer::profile_settings_init(url_to_server);
          int response_code = ExternalServer::http_get();
          if (response_code > 0){
            DynamicJsonDocument sets(150);
            deserializeJson(sets, ExternalServer::http_get_string());
            ExternalServer::close_http_session();
  
            for (uint8_t i = 0; i < NUM_DAYS; i++){
              String num_of_day = "d";
              num_of_day += String(i);
              if (sets[num_of_day] == S_WEEKDAYS){
                BoilerProfile::set_config_day_profile(i, PRESET_WEEKDAY);
              } else if (sets[num_of_day] == S_WEEKEND){
                BoilerProfile::set_config_day_profile(i, PRESET_WEEKEND);
              } else if (sets[num_of_day] == S_CUSTOM){
                BoilerProfile::set_config_day_profile(i, PRESET_CUSTOM);
              } else if (sets[num_of_day] == S_NOTFREEZE){
                BoilerProfile::set_config_day_profile(i, PRESET_NOTFREEZE);
              }
            }
          } else {
            ExternalServer::serial_error_report(url_to_server, response_code);
          }
        }
        
        BoilerProfile::save_configuration();
        
      } else {
        ExternalServer::close_http_session();
        ExternalServer::serial_error_report(url_to_server, response_code);
        ExternalServer::set_connected_to_server(false);
      }
    }
  } 
}

void ExternalServer::profile_settings_init(String url) {
  ExternalServer::profile_settings_init(url);
  int response_code = ExternalServer::http_get();
  if (response_code > 0) {
    DynamicJsonDocument sets(150);
    deserializeJson(sets, ExternalServer::http_get_string());
    ExternalServer::close_http_session();

    for (uint8_t i = 0; i < NUM_PERIODS; i++) {
      String num_of_period = "h";
      num_of_period += String(i);
      uint8_t period_temp = sets[num_of_period];
      if (!(period_temp >= WATER_TEMP_MIN && period_temp <= WATER_TEMP_MAX)) {
        period_temp = WATER_TEMP_MIN;
    }
    BoilerProfile::set_day_preset(PRESET_WEEKDAY, i, period_temp);
  }

  } else {
    ExternalServer::serial_error_report(url, response_code);
  }
}

void ExternalServer::serial_error_report(String target_url, int response_code) {
  Serial.print(F("path: "));
  Serial.println(target_url);
  Serial.print(F("responseCode: "));
  Serial.println(ExternalServer::get_http_error(response_code));
}

void ExternalServer::start_http(String url_to_server) {
  ExternalServer::http.begin(url_to_server);
  ExternalServer::http.addHeader(ExternalServer::HEADER_TYPE, ExternalServer::JSON_HEADER);
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
  return ExternalServer::WebServerAddr;
}

void ExternalServer::http_send_json(String json_string) {
  ExternalServer::http.PUT(json_string);
}

void ExternalServer::close_http_session() {
  ExternalServer::http.end();
}

int ExternalServer::http_get() {
  return ExternalServer::http.GET();
}

String ExternalServer::http_get_string() {
  return ExternalServer::http.getString();
}

String ExternalServer::get_http_error(int response_code) {
  return ExternalServer::http.errorToString(response_code);
}

void ExternalServer::set_connected_to_server(bool value) {
  ExternalServer::connected_to_server = value;
}
