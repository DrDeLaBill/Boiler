#include "ExternalServer.h"

uint32_t ExternalServer::last_time_http = 0;
bool ExternalServer::got_new_wifi_settings = false;
bool ExternalServer::connected_to_server = DISCONNECTED;
HTTPClient ExternalServer::http;
const char* ExternalServer::HEADER_TYPE = "Content-Type";
const char* ExternalServer::JSON_HEADER = "application/json";
const char* ExternalServer::WebServerAddr = "http://192.168.0.111";
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
    Serial.print(F("Send setttings to server. Server URL: "));
    Serial.println(url_to_server);
    ExternalServer::start_http(url_to_server);
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
      return;
    }
    ExternalServer::close_http_session();
    doc.clear();

    // отправляем настройки всех профилей
    url_to_server = path_to_server + "/profile/";

    for (uint8_t j = 0; j < NUM_PRESETS; j++){
      String url_profile = url_to_server + DisplayManager::presets[j];
      ExternalServer::start_http(url_profile);
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
        return; 
      }
      ExternalServer::close_http_session();
    }
    doc.clear();

    // отправляем профиль на неделю
    url_to_server = path_to_server + "/week";
    ExternalServer::start_http(url_to_server);
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
      return;
    }
    ExternalServer::close_http_session();
    doc.clear();
}

//TODO: сократить повторяющиеся части кода в функции (также для profile_settings_init())
//TODO:  проверить, не перепутались ли функции при измении их на статические, к примеру при замене http_get_string() и http_get(), тк они очень похожи
void ExternalServer::check_settings() {
  // отправляем статус и запрашиваем настройки раз в минуту.
  if (millis() - ExternalServer::last_time_http < WEB_REQUESTS_PERIOD){
    return;
  }
  ExternalServer::last_time_http = millis();
  // Проверяем подключение к интернету.
  if (ExternalServer::got_new_wifi_settings){
    NetworkManager::connect_to_wifi();
    ExternalServer::got_new_wifi_settings = false;
    return;
  }
  if (!NetworkManager::is_wifi_connected()) {
    Serial.println(F("No internet connection"));
    return;
  }
  int response_code;
  if (ExternalServer::get_new_wifi_settings() == SETS_NOT_SENDED){
    ExternalServer::send_settings_to_server();
  }
  
  String path_to_server = String(ExternalServer::get_web_server_address()) 
                        + "/boiler/" 
                        + String(BoilerProfile::get_boiler_id());
  // отправляем текущий статус котла
  String url_to_server = path_to_server + "/status";
  ExternalServer::start_http(url_to_server);
  String message = "{\n";
  uint8_t num_preset = BoilerProfile::get_profile_for_week_day();
  // ExternalServer::profile_settings_init(url_to_server);
  message += " \"temp\": " + String(TemperatureSensor::get_current_temperature()) 
          + ",\n \"target_temp\": " + String(BoilerProfile::get_target_temp())
          + ",\n \"current_profile\": \"" + DisplayManager::presets[num_preset] + "\",\n";
  message += " \"mode\": \"";
  if (BoilerProfile::is_set_session_boiler_mode(MODE_AIR)) {
    message += String(S_SETPOINT);
  } else if (BoilerProfile::is_set_session_boiler_mode(MODE_PROFILE)) {
    message += String(S_PROFILE);
  } else if (BoilerProfile::is_set_session_boiler_mode(MODE_WATER)) {
    message += String(S_SETPOINTWATER);
  }
  message += "\",\n \"errors\": [";
  for(int i = 0; i < ERRORS_COUNT; i++) 
  {
    if (ErrorService::errors_list[i] == 0) {
      break;
    }
    message = String(ErrorService::errors_list[i]);
    if (i < ERRORS_COUNT - 1 && ErrorService::errors_list[i] != 0) {
      message += ",";
    }
  }
  message += "]\n}";
  response_code = ExternalServer::http_send_json(message);
  Serial.println(message);
  ExternalServer::close_http_session();
  if (response_code > 0){
      ExternalServer::connected_to_server = CONNECTED;
  } else {
    Serial.println(F("The server is unavailable"));
    ExternalServer::connected_to_server = SETS_NOT_SENDED;
    Serial.print(F("path: "));
    Serial.println(url_to_server);
    Serial.print(F("response_code: "));
    Serial.println(ExternalServer::get_http_error(response_code));
    return;
  }

  // проверяем есть ли новые настройки
  DynamicJsonDocument doc(150);
  url_to_server = path_to_server + "/changed_by_client";
  ExternalServer::profile_settings_init(url_to_server);
  response_code = ExternalServer::http_get();

  if (response_code <= 0) {
    ExternalServer::close_http_session();
    ExternalServer::serial_error_report(url_to_server, response_code);
    ExternalServer::set_connected_to_server(false);
    return;
  }

  ExternalServer::set_connected_to_server(true);
  deserializeJson(doc, ExternalServer::http_get_string());
  ExternalServer::close_http_session();
  // проверяем обновления по всем настройкам
  if (doc["mode"]){
    // есть изменения в режиме работы
    url_to_server = path_to_server + "/mode";
    ExternalServer::profile_settings_init(url_to_server);
    int response_code = ExternalServer::http_get();
    if (response_code > 0){
      DynamicJsonDocument set_mode(50);
      deserializeJson(set_mode, ExternalServer::http_get_string());
      ExternalServer::close_http_session();

      String target_mode = set_mode.as<String>();
      if (target_mode == S_SETPOINTWATER){
        // работаем по теплоносителю
        BoilerProfile::set_boiler_mode(MODE_WATER);
      }  else if (ErrorService::is_set_error(ERROR_RADIOSENSOR)) {
        // Если отвален датчик работаем по теплоносителю
        if (BoilerProfile::session_boiler_mode != MODE_WATER) {
          BoilerProfile::session_boiler_mode = MODE_WATER;
          BoilerProfile::session_target_temp_int = (uint8_t)TemperatureSensor::get_current_temp_water();
          TemperatureSensor::set_current_temp_like_water_temp();
        }
      } else if (target_mode == S_SETPOINT){
        // работаем по воздуху
        BoilerProfile::set_boiler_mode(MODE_AIR);
      }  else if (target_mode == S_PROFILE){
        // работаем по термопрофилю
        BoilerProfile::set_boiler_mode(MODE_PROFILE);
      }
      BoilerProfile::save_configuration();
    } else {
      ExternalServer::serial_error_report(url_to_server, response_code);
    }
  }
  
  if (doc["target_temp"]) {
    url_to_server = path_to_server + "/target_temp";
    ExternalServer::profile_settings_init(url_to_server);
    int response_code = ExternalServer::http_get();
    if (response_code > 0){
      DynamicJsonDocument set_temp(100);
      deserializeJson(set_temp, ExternalServer::http_get_string());
      ExternalServer::close_http_session();
      int target_temp = set_temp.as<int>();
      if (!(target_temp >= WATER_TEMP_MIN && target_temp <= WATER_TEMP_MAX)) {
        target_temp = WATER_TEMP_MIN;
      }
      BoilerProfile::set_target_temp(target_temp);
      BoilerProfile::save_configuration();
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
      BoilerProfile::save_configuration();
    } else {
      ExternalServer::serial_error_report(url_to_server, response_code);
    }
  }
}

void ExternalServer::profile_settings_init(String url) {
  ExternalServer::start_http(url);
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
  Serial.print(F("[HTTP_ERROR]: "));
  Serial.println(target_url);
  Serial.print(F("responseCode: "));
  Serial.println(ExternalServer::get_http_error(response_code));
}

void ExternalServer::start_http(String url_to_server) {
  Serial.print(F("Request to: "));
  Serial.println(url_to_server);
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

//TODO: добавить лог ответа, ошибок
int ExternalServer::http_send_json(String json_string) {
  Serial.print(F("[HTTP_PUT] "));
  Serial.println(json_string);
  return ExternalServer::http.PUT(json_string);
}

void ExternalServer::close_http_session() {
  ExternalServer::http.end();
}

int ExternalServer::http_get() {
  Serial.println(F("[HTTP_GET]"));
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
