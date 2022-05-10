#include "InternalServer.h"

AsyncWebServer server(80);
AsyncCallbackJsonWebHandler* handler;
  
class OneParamRewrite : public AsyncWebRewrite {
  protected:
    String _urlPrefix;
    int _paramIndex;
    String _paramsBackup;
  public:
  OneParamRewrite(const char* from, const char* to)
    : AsyncWebRewrite(from, to) {

      _paramIndex = _from.indexOf('{');

      if( _paramIndex >=0 && _from.endsWith("}")) {
        _urlPrefix = _from.substring(0, _paramIndex);
        int index = _params.indexOf('{');
        if(index >= 0) {
          _params = _params.substring(0, index);
        }
      } else {
        _urlPrefix = _from;
      }
      _paramsBackup = _params;
  }

  bool match(AsyncWebServerRequest *request) override {
    if(request->url().startsWith(_urlPrefix)) {
      if(_paramIndex >= 0) {
        _params = _paramsBackup + request->url().substring(_paramIndex);
      } else {
        _params = _paramsBackup;
      }
      return true;
    } else {
      return false;
    }
  }
};

// handle the upload of the firmware
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    // handle upload and update
    if (!index) {
        Serial.printf("Update: %s\n", filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN))
        { //start with max available size
            Update.printError(Serial);
        }
    }

    /* flashing firmware to ESP*/
    if (len) {
        Update.write(data, len);
    }

    if (final) {
        if (Update.end(true)) { //true to set the size to the current progress
            Serial.printf("Update Success: %ub written\nRebooting...\n", index+len);
        } else {
            Update.printError(Serial);
        }
    }
}

void start_internal_server() {
  Serial.println(F("Internal server settings start"));
  
  // страница для загрузки новой прошивки
  server.on("/firmware_upload", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("[HTTP_GET] /firmware_upload.html");
    request->send(SPIFFS, "/firmware_update.html", "text/html");
  });

  //подключение библиотеки jQuery
  server.on("/jquery-3.6.0.min.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/jquery-3.6.0.min.js", "text/javascript");
    Serial.println("[HTTP_GET] jquery-3.6.0.min.js");
  });

  //подключение библиотеки JS скрипта 
  server.on("/ajax_firmware_upload.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/ajax_firmware_upload.js", "text/javascript");
    Serial.println("[HTTP_GET] ajax_firmware_upload.js");
  });
    
  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (!Update.hasError()) {
      AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "OK");
      response->addHeader("Connection", "close");
      request->send(response);
      Serial.println("[HTTP_POST] ERROR update firmware (request error, wrong form)");
      ESP.restart();
    } else {
      AsyncWebServerResponse *response = request->beginResponse(500, "text/plain", "ERROR");
      response->addHeader("Connection", "close");
      request->send(response);
      Serial.println("[HTTP_POST] update firmware");
    } 
  }, handleUpload);

//  server.on("/id", HTTP_POST, [](AsyncWebServerRequest *request) {
//    String message;
//    if (request->hasParam("ID", true)) {
//        message = request->getParam("ID", true)->value();
//        message.toCharArray(BoilerProfile::boiler_configuration.boiler_id, ID_MAX_SIZE);
//        BoilerProfile::save_configuration();
//        Serial.println("Got new ID: ");
//        Serial.println(String(BoilerProfile::boiler_configuration.boiler_id));
//    } else {
//        message = "No message sent";
//    }
//    request->send(200, "text/plain", "Hello, POST: " + message);
//    Serial.println("[HTTP_POST] get id");
//  });
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
//    if(!request->authenticate(http_login, http_pass))
//        return request->requestAuthentication();
    request->send(200, "text/plain", "Boiler server");
    Serial.println("[HTTP_GET] boiler server");
  });


  handler = new AsyncCallbackJsonWebHandler("/client/names", [](AsyncWebServerRequest *request, JsonVariant &json) {
    // сохранение нового имени котла

//    if(!request->authenticate(http_login, http_pass))
//        return request->requestAuthentication();

    int array_size = json.size();
//    Serial.print("array_size: ");
//    Serial.println(array_size);

    String message = "{\"\":\"\"}";
    for (uint8_t i = 0; i < array_size; i++) {
//      Serial.println(json[i].as<String>());
      if (json[i].as<String>() == String(BoilerProfile::boiler_configuration.boiler_id)) {
        message = "{\"" + String(BoilerProfile::boiler_configuration.boiler_id) + "\":\"" + String(BoilerProfile::boiler_configuration.boiler_name) + "\"}";
      }
    }

    request->send(200, "text/plain", message);
  });
  server.addHandler(handler);

  String url_path = "/client/" + String(BoilerProfile::boiler_configuration.boiler_id) + "/name";
  handler = new AsyncCallbackJsonWebHandler(url_path.c_str(), [](AsyncWebServerRequest *request, JsonVariant &json) {
    // сохранение нового имени котла

//    if(!request->authenticate(http_login, http_pass))
//        return request->requestAuthentication();

    Serial.print("got new boiler name: ");
    Serial.println(String(BoilerProfile::boiler_configuration.boiler_name));
    
    if (String(BoilerProfile::boiler_configuration.boiler_name) != json.as<String>()) {
      json.as<String>().toCharArray(BoilerProfile::boiler_configuration.boiler_name, NAME_MAX_SIZE);
      BoilerProfile::save_configuration();
    }

    request->send(200, "text/plain", "");
  });
  server.addHandler(handler);

  url_path = "/client/" + String(BoilerProfile::boiler_configuration.boiler_id) + "/router_connection";
  server.on(url_path.c_str(), HTTP_GET, [] (AsyncWebServerRequest *request) {
    // получение ssid и пароля для подключения к роутеру
    
//    if(!request->authenticate(http_login, http_pass))
//        return request->requestAuthentication();
    request->send(200, "text/plain", "{\"ssid\": \"" + NetworkManager::current_ssid + "\", \"password\": \"" + NetworkManager::current_pass + "\"}");
  });

  handler = new AsyncCallbackJsonWebHandler(url_path.c_str(), [](AsyncWebServerRequest *request, JsonVariant &json) {
    // сохранение SSID & PSWD локальной сети

//    if(!request->authenticate(http_login, http_pass))
//        return request->requestAuthentication();

    if (NetworkManager::current_ssid != json["ssid"].as<String>() || NetworkManager::current_pass != json["password"].as<String>()) {
      NetworkManager::current_ssid = json["ssid"].as<String>();
      NetworkManager::current_pass = json["password"].as<String>();
      Serial.println("got new settings: SSID & PSWD");
      Serial.print("ssid: ");
      Serial.println(NetworkManager::current_ssid);
      Serial.print("pswd: ");
      Serial.println(NetworkManager::current_pass);
      BoilerProfile::save_configuration();
      ExternalServer::got_new_wifi_settings = true;
    }

    request->send(200, "text/plain", "");
  });
  server.addHandler(handler);

  String url_path_from = "/client/" + String(BoilerProfile::boiler_configuration.boiler_id) + "/profile/{profile}";
  String url_path_to = "/client/" + String(BoilerProfile::boiler_configuration.boiler_id) + "/profile?name={profile}";
  server.addRewrite(new OneParamRewrite(url_path_from.c_str(), url_path_to.c_str()));

  url_path = "/client/" + String(BoilerProfile::boiler_configuration.boiler_id) + "/profile";
  server.on(url_path.c_str(), HTTP_GET, [] (AsyncWebServerRequest *request) {
    // возвращаем пресет термопрофиля в зависимости от имени
    
//    if(!request->authenticate(http_login, http_pass))
//        return request->requestAuthentication();
        
    if (request->hasParam(S_NAME)) {
      String preset_name = request->getParam(S_NAME)->value();
      String message = "{\n";

      for (uint8_t i = 0; i < NUM_PERIODS; i++) {
        message += "\"h" + String(i) + "\": ";
        
        if (preset_name == S_WEEKDAYS) {
           message += String(BoilerProfile::boiler_configuration.presets[PRESET_WEEKDAY][i]);
        } else if (preset_name == S_WEEKEND) {
          message += String(BoilerProfile::boiler_configuration.presets[PRESET_WEEKEND][i]);
        } else if (preset_name == S_CUSTOM) {
          message += String(BoilerProfile::boiler_configuration.presets[PRESET_CUSTOM][i]);
        } else if (preset_name == S_NOTFREEZE) {
          message += String(BoilerProfile::boiler_configuration.presets[PRESET_NOTFREEZE][i]);
        } else {
          message = "Unknown profile";
          break;
        }
        
        if (i < NUM_PERIODS-1) message +=  ",\n";
        else message += "\n}";
      }
      
      request->send(200, "text/plain", message);

    }
  });
  
  handler = new AsyncCallbackJsonWebHandler(url_path.c_str(), [](AsyncWebServerRequest *request, JsonVariant &json) {
    // сохранение пресета термопрофиля

//    if(!request->authenticate(http_login, http_pass))
//        return request->requestAuthentication();

    if (request->hasParam(S_NAME)) {
      String preset_name = request->getParam(S_NAME)->value();

      for (uint8_t i = 0; i < NUM_PERIODS; i++) {
        String num_of_period = "h";
        num_of_period += String(i);
        uint8_t period_temp = json[num_of_period].as<unsigned char>();
        if (period_temp >= WATER_TEMP_MIN && period_temp <= WATER_TEMP_MAX);
        else period_temp = WATER_TEMP_MIN;
        if (preset_name == S_WEEKDAYS) {
          BoilerProfile::boiler_configuration.presets[PRESET_WEEKDAY][i] = period_temp;
        } else if (preset_name == S_WEEKEND) {
          BoilerProfile::boiler_configuration.presets[PRESET_WEEKEND][i] = period_temp;
        } else if (preset_name == S_CUSTOM) {
          BoilerProfile::boiler_configuration.presets[PRESET_CUSTOM][i] = period_temp;
        } else if (preset_name == S_NOTFREEZE) {
          BoilerProfile::boiler_configuration.presets[PRESET_NOTFREEZE][i] = period_temp;
        } 
      }
      
      BoilerProfile::save_configuration();
    }
    request->send(200, "text/plain", "");
  });
  server.addHandler(handler);

  url_path = "/client/" + String(BoilerProfile::boiler_configuration.boiler_id) + "/status";
  server.on(url_path.c_str(), HTTP_GET, [] (AsyncWebServerRequest *request) {
    // возвращаем текущее состояние котла
    
//    if(!request->authenticate(http_login, http_pass))
//        return request->requestAuthentication();
      
    String message = "";
      
    message = "{\n\"router_connection\": ";
    if (WiFi.status() == WL_CONNECTED) message += "true";
    else message += "false";
    message += ",\n\"internet\": false,\n\"remote_server_connection\": ";
    if (ExternalServer::connected_to_server == DISCONNECTED) {
      message += "false";
    } else {
      message += "true";
    }
    message += ",\n\"temp\": " +
    String(TemperatureSensor::current_temp) + ",\n\"current_profile\": \"";

    switch(BoilerProfile::boiler_configuration.profile[ClockRTC::clock_get_day_of_week()]) {
      case PRESET_WEEKDAY:
        message += String(S_WEEKDAYS);
        break;

      case PRESET_WEEKEND:
        message += String(S_WEEKEND);
        break;

      case PRESET_CUSTOM:
        message += String(S_CUSTOM);
        break;

      case PRESET_NOTFREEZE:
        message += String(S_NOTFREEZE);
        break;

      default:
        message += "";
        break;
    }

    message += "\",\n\"errors\": [";

    if (TemperatureSensor::radio_connected == RADIO_LOST) {
      // внешний датчик отвалился
      message += "\"no_sensor\"";
    }

//    if (BoilerProfile::session_boiler_mode == MODE_WATER) {
//      // котел работает по теплоносителю
//      message += S_SETPOINTWATER;
//    } else if (BoilerProfile::session_boiler_mode == MODE_PROFILE) {
//      // котел работает по термопрофилю
//      message += S_PROFILE;
//    } else if (BoilerProfile::session_boiler_mode == MODE_AIR) {
//      // котел работает по воздуху
//      message += S_SETPOINT;
//    }

    message += "]\n}";
        
    request->send(200, "text/plain", message);
    
  });

  url_path = "/client/" + String(BoilerProfile::boiler_configuration.boiler_id) + "/mode";
  server.on(url_path.c_str(), HTTP_GET, [] (AsyncWebServerRequest *request) {
    // запрос текущего режима работы - по воздуху, теплоносителю, или термопрофиль
    
//    if(!request->authenticate(http_login, http_pass))
//        return request->requestAuthentication();

    String message = "\"";

    if (BoilerProfile::session_boiler_mode == MODE_WATER) {
      // режим работы по теплоносителю
      message += S_SETPOINTWATER;
      message += "\"";
      request->send(200, "text/plain", message);
    } else if (BoilerProfile::session_boiler_mode == MODE_PROFILE) {
      // режим работы по термопрофилю
      message += S_PROFILE;
      message += "\"";
      request->send(200, "text/plain", message);
    } else if (BoilerProfile::session_boiler_mode == MODE_AIR) {
      // режим работы уставка по воздуху
      message += S_SETPOINT;
      message += "\"";
      request->send(200, "text/plain", message);
    }
  });

  handler = new AsyncCallbackJsonWebHandler(url_path.c_str(), [](AsyncWebServerRequest *request, JsonVariant &json) {
    // установка режима работы - по воздуху, по теплоносителю или термопрофиль
    
//    if(!request->authenticate(http_login, http_pass))
//        return request->requestAuthentication();
        
    request->send(200, "text/plain", "");
    String target_mode = json.as<String>();
//    Serial.print("target_mode: ");
//    Serial.println(target_mode);
    if (target_mode == S_SETPOINT) {
      // работаем по воздуху
      BoilerProfile::set_boiler_mode(MODE_AIR);
    } else if (target_mode == S_SETPOINTWATER) {
      // работаем по теплоносителю
      BoilerProfile::set_boiler_mode(MODE_WATER);
    } else if (target_mode == S_PROFILE) {
      // работает по термопрофилю
      BoilerProfile::set_boiler_mode(MODE_PROFILE);
    }
    
  });
  server.addHandler(handler);

  url_path = "/client/" + String(BoilerProfile::boiler_configuration.boiler_id) + "/target_temp";
  server.on(url_path.c_str(), HTTP_GET, [] (AsyncWebServerRequest *request) {
    // запрос текущей установленной температуры
    
//    if(!request->authenticate(http_login, http_pass))
//        return request->requestAuthentication();

    request->send(200, "text/plain", String(BoilerProfile::get_target_temp()));
  });

  server.on(url_path.c_str(), HTTP_PUT, 
      [] (AsyncWebServerRequest *request) {},
      [] (AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {},
      [] (AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    // установка текущей температуры
    
//    if(!request->authenticate(http_login, http_pass))
//        return request->requestAuthentication();

    String str = String((char*)data);
    uint8_t need_temp = str.substring(0, len).toInt();
//    Serial.print("need_temp: ");
//    Serial.println(need_temp);
    if (need_temp >= WATER_TEMP_MIN && need_temp <= WATER_TEMP_MAX) {
      BoilerProfile::set_target_temp(need_temp);
    }

    request->send(200, "text/plain", "");
  });

  url_path = "/client/" + String(BoilerProfile::boiler_configuration.boiler_id) + "/datetime";
  handler = new AsyncCallbackJsonWebHandler(url_path.c_str(), [](AsyncWebServerRequest *request, JsonVariant &json) {
    // сохранение даты/времени и часового пояса

//    if(!request->authenticate(http_login, http_pass))
//        return request->requestAuthentication();
    
    uint64_t datetime = json.as<unsigned long long>();
    uint8_t timezone = 0; // default = utc. Решили, что клиент сам будет определять локаль и отправлять соответствующее время.
    Serial.print("datetime: ");
    Serial.println((uint32_t)datetime);
    Serial.print("timezone: ");
    Serial.println(timezone);
    
    ClockRTC::clock_set_time(&datetime, &timezone);  
    request->send(200, "text/plain", "");
  });
  server.addHandler(handler);

  url_path = "/client/" + String(BoilerProfile::boiler_configuration.boiler_id) + "/week";
  server.on(url_path.c_str(), HTTP_GET, [] (AsyncWebServerRequest *request) {
    // возвращаем недельный набор пресетов
    
//    if(!request->authenticate(http_login, http_pass))
//        return request->requestAuthentication();
        
    String message = "";

    message = "{\n";

    for (uint8_t i = 0; i < NUM_DAYS; i++) {
      message += "\"d" + String(i) + "\": \"";
      
      switch (BoilerProfile::boiler_configuration.profile[i]) {

        case PRESET_WEEKDAY:
          message += String(S_WEEKDAYS);
          break;

        case PRESET_WEEKEND:
          message += String(S_WEEKEND);
          break;

        case PRESET_CUSTOM:
          message += String(S_CUSTOM);
          break;

        case PRESET_NOTFREEZE:
          message += String(S_NOTFREEZE);
          break;

        default:
          message += "\"\"";
          break;

      }
      if (i != NUM_DAYS-1)
        message += "\",\n";
      else
        message += "\"\n}";
    }

    request->send(200, "text/plain", message);
    
  });

  handler = new AsyncCallbackJsonWebHandler(url_path.c_str(), [](AsyncWebServerRequest *request, JsonVariant &json) {
    // сохранение профиля пресетов на неделю

//    if(!request->authenticate(http_login, http_pass))
//        return request->requestAuthentication();

    for (uint8_t i = 0; i < NUM_DAYS; i++) {
      String num_of_day = "d";
      num_of_day += String(i);
      String day_preset = json[num_of_day].as<String>();
      if (day_preset == S_WEEKDAYS) {
        BoilerProfile::boiler_configuration.profile[i] = PRESET_WEEKDAY;
      } else if (day_preset == S_WEEKEND) {
        BoilerProfile::boiler_configuration.profile[i] = PRESET_WEEKEND;
      } else if (day_preset == S_CUSTOM) {
        BoilerProfile::boiler_configuration.profile[i] = PRESET_CUSTOM;
      } else if (day_preset == S_NOTFREEZE) {
        BoilerProfile::boiler_configuration.profile[i] = PRESET_NOTFREEZE;
      }
    }
    BoilerProfile::save_configuration();
    request->send(200, "text/plain", "");
  });
  server.addHandler(handler);
   
  server.begin();
  Serial.println(F("Internal server settings end"));
}
