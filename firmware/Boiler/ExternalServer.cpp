#include "ExternalServer.h"

ExternalServer::ExternalServer() {
  bool connected_to_server = DISCONNECTED;
  bool got_new_wifi_settings = false;
}

void send_settings_to_server(void) {
  // проверяем подключение к интернету. Настройки отправляем при включении и при каждом их изменении.
  if (WiFi.status() == WL_CONNECTED) {
    
    String path_to_server = String(WebServerAddr) + "/boiler/";
    path_to_server += String(boiler_id);
    
    DynamicJsonDocument doc(150);
    int response_code;
    
    String url_to_server = path_to_server + "/settings";
    http.begin(url_to_server);
    http.setConnectTimeout(100);
    http.addHeader(HEADER_TYPE, JSON_HEADER);

    // текущий режим работы и выставленная температура
    if (user_boiler_mode == MODE_AIR) doc["mode"] = String(S_SETPOINT);
    else if (user_boiler_mode == MODE_WATER) doc["mode"] = String(S_SETPOINTWATER);
    else if (user_boiler_mode == MODE_PROFILE) doc["mode"] = String(S_PROFILE);
    doc["target_temp"] = get_target_temp();
    String send_json = "";
    serializeJson(doc, send_json);
    response_code = http.PUT(send_json);
    if (response_code > 0){
      connected_to_server = CONNECTED;
    } else {
      connected_to_server = SETS_NOT_SENDED;
      Serial.print(F("path: "));
      Serial.println(url_to_server);
      Serial.print(F("response_code: "));
      Serial.println(http.errorToString(response_code));
    }
    http.end();
    doc.clear();

    // отправляем настройки всех профилей
    url_to_server = path_to_server + "/profile/";

    for (uint8_t j = 0; j < NUM_PRESETS; j++){
      String url_profile = url_to_server + presets[j];
      http.begin(url_profile);
      http.setConnectTimeout(100);
      http.addHeader(HEADER_TYPE, JSON_HEADER);
      for (uint8_t i = 0; i < NUM_PERIODS; i++){
        String p_period = "h";
        p_period += String(i);
        doc[p_period] = BoilerCfg.presets[j][i];
      }
      send_json = "";
      serializeJson(doc, send_json);
      response_code = http.PUT(send_json);
      if (response_code > 0){
        connected_to_server = CONNECTED;
      } else {
        connected_to_server = SETS_NOT_SENDED;
        Serial.print(F("path: "));
        Serial.println(url_to_server);
        Serial.print(F("response_code: "));
        Serial.println(http.errorToString(response_code));
      }
      http.end();
    }
    doc.clear();

    // отправляем профиль на неделю
    url_to_server = path_to_server + "/week";
    http.begin(url_to_server);
    http.setConnectTimeout(100);
    http.addHeader(HEADER_TYPE, JSON_HEADER);
    for (uint8_t i = 0; i < NUM_DAYS; i++){
      String p_day = "d";
      p_day += String(i);
      doc[p_day] = presets[BoilerCfg.profile[i]];
    }
    send_json = "";
    serializeJson(doc, send_json);
    response_code = http.PUT(send_json);
    if (response_code > 0){
      connected_to_server = CONNECTED;
    } else {
      connected_to_server = SETS_NOT_SENDED;
      Serial.print(F("path: "));
      Serial.println(url_to_server);
      Serial.print(F("response_code: "));
      Serial.println(http.errorToString(response_code));
    }
    http.end();
    doc.clear();
  }
}

void check_new_settings(void){
  static uint32_t last_time_http = 0;

  if (got_new_wifi_settings){
    got_new_wifi_settings = false;
    connect_to_wifi();
  }

  if (WiFi.status() == WL_CONNECTED){
    // отправляем статус и запрашиваем настройки раз в минуту.
    
    if (millis() - last_time_http >= WEB_REQUESTS_PERIOD){
      last_time_http = millis();
      int response_code;

      if (connected_to_server == SETS_NOT_SENDED){
        send_settings_to_server();
      }
  
      String path_to_server = String(WebServerAddr) + "/boiler/";
      path_to_server += String(boiler_id);
  
      DynamicJsonDocument doc(150);
  
      // отправляем текущий статус котла
      String url_to_server = path_to_server + "/status";
      http.begin(url_to_server);
      http.addHeader(HEADER_TYPE, JSON_HEADER);
      doc["temp"] = current_temp;
      doc["target_temp"] = get_target_temp();
      uint8_t num_preset = BoilerCfg.profile[clock_get_day_of_week()];
      doc["current_profile"] = presets[num_preset];
      if (user_boiler_mode == MODE_AIR) doc["current_mode"] = String(S_SETPOINT);
      else if (user_boiler_mode == MODE_PROFILE) doc["current_mode"] = String(S_PROFILE);
      else if (user_boiler_mode == MODE_WATER) doc["current_mode"] = String(S_SETPOINTWATER);
      String send_json = "";
      serializeJson(doc, send_json);
      http.PUT(send_json);
      http.end();
      doc.clear();
  
      // проверяем есть ли новые настройки
      url_to_server = path_to_server + "/changed_by_client";
      http.begin(url_to_server);
      response_code = http.GET();
      if (response_code > 0){
        connected_to_server = true;
        deserializeJson(doc, http.getString());
        http.end();
        // проверяем обновления по всем настройкам
        if (doc["settings"] == true){
          // есть изменения в режиме работы
          
          url_to_server = path_to_server + "/settings";
          http.begin(url_to_server);
          int response_code = http.GET();
            if (response_code > 0){
              DynamicJsonDocument sets(100);
              deserializeJson(sets, http.getString());
              http.end();
              
              if (sets["mode"] == S_SETPOINT){
                // работаем по воздуху
                set_boiler_mode(MODE_AIR);
              } else if (sets["mode"] == S_SETPOINTWATER){
                // работаем по теплоносителю
                set_boiler_mode(MODE_WATER);
              } else if (sets["mode"] == S_PROFILE){
                // работаем по термопрофилю
                set_boiler_mode(MODE_PROFILE);
              }
              uint8_t need_temp = sets["target_temp"];
              if (need_temp >= WATER_TEMP_MIN && need_temp <= WATER_TEMP_MAX);
              else  need_temp = WATER_TEMP_MIN;
              set_target_temp(need_temp);
            } else {
              Serial.print(F("path: "));
              Serial.println(url_to_server);
              Serial.print(F("responseCode: "));
              Serial.println(http.errorToString(response_code));
            }
        }
  
        if (doc["profile_weekdays"] == true){
          // есть изменения в профиле
  
          url_to_server = path_to_server + "/profile/weekdays";
          http.begin(url_to_server);
          int response_code = http.GET();
            if (response_code > 0){
              DynamicJsonDocument sets(150);
              deserializeJson(sets, http.getString());
              http.end();
  
              for (uint8_t i = 0; i < NUM_PERIODS; i++){
                String num_of_period = "h";
                num_of_period += String(i);
                uint8_t period_temp = sets[num_of_period];
                if (period_temp >= WATER_TEMP_MIN && period_temp <= WATER_TEMP_MAX);
                else period_temp = WATER_TEMP_MIN;
                BoilerCfg.presets[PRESET_WEEKDAY][i] = period_temp;
              }
              
            } else {
              Serial.print(F("path: "));
              Serial.println(url_to_server);
              Serial.print(F("responseCode: "));
              Serial.println(http.errorToString(response_code));
            }
        }
  
        if (doc["profile_weekend"] == true){
          // есть изменения в профиле
  
          url_to_server = path_to_server + "/profile/weekend";
          http.begin(url_to_server);
          int response_code = http.GET();
            if (response_code > 0){
              DynamicJsonDocument sets(150);
              deserializeJson(sets, http.getString());
              http.end();
  
              for (uint8_t i = 0; i < NUM_PERIODS; i++){
                String num_of_period = "h";
                num_of_period += String(i);
                uint8_t period_temp = sets[num_of_period];
                if (period_temp >= WATER_TEMP_MIN && period_temp <= WATER_TEMP_MAX);
                else period_temp = WATER_TEMP_MIN;
                BoilerCfg.presets[PRESET_WEEKEND][i] = period_temp;
              }
              
            } else {
              Serial.print("path: ");
              Serial.println(url_to_server);
              Serial.print(F("responseCode: "));
              Serial.println(http.errorToString(response_code));
            }
        }
  
        if (doc["profile_custom"] == true){
          // есть изменения в профиле
  
          url_to_server = path_to_server + "/profile/custom";
          http.begin(url_to_server);
          int response_code = http.GET();
            if (response_code > 0){
              DynamicJsonDocument sets(150);
              deserializeJson(sets, http.getString());
              http.end();
  
              for (uint8_t i = 0; i < NUM_PERIODS; i++){
                String num_of_period = "h";
                num_of_period += String(i);
                uint8_t period_temp = sets[num_of_period];
                if (period_temp >= WATER_TEMP_MIN && period_temp <= WATER_TEMP_MAX);
                else period_temp = WATER_TEMP_MIN;
                BoilerCfg.presets[PRESET_CUSTOM][i] = period_temp;
              }
              
            } else {
              Serial.print(F("path: "));
              Serial.println(url_to_server);
              Serial.print(F("responseCode: "));
              Serial.println(http.errorToString(response_code));
            }
        }
  
        if (doc["profile_notfreeze"] == true){
          // есть изменения в профиле
  
          url_to_server = path_to_server + "/profile/notfreeze";
          http.begin(url_to_server);
          int response_code = http.GET();
            if (response_code > 0){
              DynamicJsonDocument sets(150);
              deserializeJson(sets, http.getString());
              http.end();
  
              for (uint8_t i = 0; i < NUM_PERIODS; i++){
                String num_of_period = "h";
                num_of_period += String(i);
                uint8_t period_temp = sets[num_of_period];
                if (period_temp >= WATER_TEMP_MIN && period_temp <= WATER_TEMP_MAX);
                else period_temp = WATER_TEMP_MIN;
                BoilerCfg.presets[PRESET_NOTFREEZE][i] = period_temp;
              }
              
            } else {
              Serial.print(F("path: "));
              Serial.println(url_to_server);
              Serial.print(F("responseCode: "));
              Serial.println(http.errorToString(response_code));
            }
        }
  
        if (doc["week"] == true){
          // есть изменения в графике на неделю
  
          url_to_server = path_to_server + "/week";
          http.begin(url_to_server);
          int response_code = http.GET();
          if (response_code > 0){
            DynamicJsonDocument sets(150);
            deserializeJson(sets, http.getString());
            http.end();
  
            for (uint8_t i = 0; i < NUM_DAYS; i++){
              String num_of_day = "d";
              num_of_day += String(i);
              if (sets[num_of_day] == S_WEEKDAYS){
                BoilerCfg.profile[i] = PRESET_WEEKDAY;
              } else if (sets[num_of_day] == S_WEEKEND){
                BoilerCfg.profile[i] = PRESET_WEEKEND;
              } else if (sets[num_of_day] == S_CUSTOM){
                BoilerCfg.profile[i] = PRESET_CUSTOM;
              } else if (sets[num_of_day] == S_NOTFREEZE){
                BoilerCfg.profile[i] = PRESET_NOTFREEZE;
              }
            }
          } else {
            Serial.print(F("path: "));
            Serial.println(url_to_server);
            Serial.print(F("responseCode: "));
            Serial.println(http.errorToString(response_code));
          }
        }
        
        save_cfg();
        
      } else {
        http.end();
        Serial.print(F("path: "));
        Serial.println(url_to_server);
        Serial.print(F("responseCode: "));
        Serial.println(http.errorToString(response_code));
        connected_to_server = false;
      }
    }
  }
}
