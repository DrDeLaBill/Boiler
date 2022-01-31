#include "BoilerController.h"

BoilerController::BoilerController() {
  Serial.begin(115200);
  Serial.println(F("\n######################################################"));
  Serial.println(F("Initialization boiler started."));
  if(!SPIFFS.begin(true)){
    Serial.println(F("An Error has occurred while mounting SPIFFS"));
    //  "При монтировании SPIFFS произошла ошибка"
    ESP.restart();
  }
  this->display_manager = new DisplayManager();
  this->error_service = new ErrorService();
  this->network_manager = new NetworkManager();
  this->external_server = new ExternalServer();
  this->boiler_profile = new BoilerProfile();
  this->internal_server = new InternalServer();
  this->command_manager = new CommandManager();
  this->encoder_manager = new EncoderManager();
  this->relay_manager = new RelayTemperature();
  this->pump_manager = new PumpManager();
  this->controller_init();
}

void BoilerController::controller_init() {
  this->work_mode = MODE_STANDBY;
  this->network_manager->set_wifi_settings(
    this->boiler_profile->get_ssid(),
    this->boiler_profile->get_pass()
  );
  // проверяем, надо ли включаться или нет.
  if (BoilerProfile::boiler_configuration.standby_flag == WORK) {
    this->work_mode = MODE_WORK;
    Serial.println("WORK MODE");
    this->pump_manager->pump_on();
  } else {
    this->work_mode = MODE_STANDBY;
    Serial.println("STANDBY MODE");
    this->display_manager->display_off();
  }
}

void BoilerController::controller_run() {
  /*
   * Загрузка значений по термопрофилю. Получение текущего времени.
   * Измерение температуры. Теплоноситель и внешний датчик.
   * Измерение температуры ТТ реле и включение вентилятора.
   * Общение с сервером. Надо поговорить об этом с Ромой.
   * Проверка исправности системы. Отображение ошибок.
   * Опрос энкодера и кнопки.
   * Рисование на дисплее.
   * 
   */
  // Проверка наличия команд через Serial порт
  this->command_manager->check_commands();

  if (this->work_mode == MODE_WORK) {
    // Проверка наличия ошибок
    this->error_service->check_failure();
    uint8_t errors[ERRORS_COUNT] = {};
    this->error_service->get_errors_list(errors);
    // проверим нагрев
    if (ErrorService::is_set_error(ERROR_NOERROR)) {
      this->boiler_profile->temperature_pid_off();
    } else {
      this->boiler_profile->temperature_pid_regulating();
    }
    // нарисуем экран
    //TODO: проверить правильность функции
    this->_fill_display_manager_configuration();
//    this->display_manager->paint();
    // измерим температуру
    this->error_service->add_error(
      this->boiler_profile->check_temperature()
    );

    // проверим температуру ТТ реле.
    // check_ssr_temp();
    // проверим энкодер
    if (this->encoder_manager->is_button_holded(this->work_mode)) {
      // если было долгое нажатие кнопки - переходим в режим ожидания
      Serial.println(F("set STANDBY MODE"));
      this->display_manager->display_off();
      this->boiler_profile->set_settings_standby(MODE_STANDBY);
      this->work_mode = MODE_STANDBY;
    }
    this->_check_external_server_sttings();
    // Поменялись ли ssid и password сети wifi
    this->_check_network_settings();
  } else if (this->work_mode == MODE_STANDBY) {
    // режим ожидания
    if (this->encoder_manager->is_button_holded(this->work_mode)) {
      // если было долгое нажатие кнопки - переходим в режим работы
      Serial.println(F("set WORK MODE"));
      this->pump_manager->pump_on();
      this->display_manager->display_on();
      this->boiler_profile->set_settings_standby(MODE_WORK);
      this->work_mode = MODE_WORK;
    }
    this->pump_manager->pump_off();
  }
  
  // Обработка ошибок
  this->error_service->init_error_actions();
}

// Совпадают ли ssid и pass BoilerProfile и NetworkManager
void BoilerController::_check_network_settings() {
  if (this->network_manager->get_ssid() != this->boiler_profile->get_ssid() 
      || this->network_manager->get_pass() != this->boiler_profile->get_pass()
  ) {
    this->boiler_profile->set_wifi_settings(
      this->network_manager->get_ssid(),
      this->network_manager->get_pass()
    );
  }
}

//TODO: для чего и откуда
//bool BoilerController::_button_holded_action() {
//  uint8_t button_state = this->encoder_manager->check_encoder(this->work_mode);
//  this->display_manager->rotary_encoder_action(
//    this->encoder_manager->get_button_rotary_state(),
//    BoilerProfile::get_session_boiler_mode()
//  );
//  if (button_state == BUTTON_HOLDED) {
//    return true;
//  } else if (button_state == BUTTON_PRESSED) {
//    this->encoder_manager->button_pressed_action();
//  }
//  return false;
//}

//TODO: сократить повторяющиеся части кода в функции (также для _external_profile_settings_init())
void BoilerController::_check_external_server_sttings() {
  if (this->external_server->get_new_wifi_settings()) {
    this->network_manager->connect_to_wifi();
    this->external_server->set_new_wifi_settings_flag(false);
  }
  // Проверяем подключение к интернету.
  if (this->network_manager->get_wifi_status() == WL_CONNECTED){
    // отправляем статус и запрашиваем настройки раз в минуту.
    
    if (millis() - ExternalServer::last_time_http >= WEB_REQUESTS_PERIOD){
      ExternalServer::last_time_http = millis();
      int response_code;
      
      if (this->external_server->get_new_wifi_settings() == SETS_NOT_SENDED){
        this->external_server->send_settings_to_server(
            BoilerProfile::get_session_boiler_mode(),
            BoilerProfile::get_target_temp(),
            this->boiler_profile->get_boiler_configuration()
        );
      }
      
      String path_to_server = String(this->external_server->get_web_server_address()) 
                            + "/boiler/" 
                            + String(this->boiler_profile->get_boiler_id());
      DynamicJsonDocument doc(150);
      
      // отправляем текущий статус котла
      String url_to_server = path_to_server + "/status";
      this->external_server->start_http(url_to_server);
      doc["temp"] = this->boiler_profile->get_current_temperature();
      doc["target_temp"] = BoilerProfile::get_target_temp();
      uint8_t num_preset = this->boiler_profile->get_profile_for_week_day();
      doc["current_profile"] = this->internal_server->get_preset(num_preset);
      if (BoilerProfile::is_mode_air())
        doc["current_mode"] = String(this->internal_server->get_s_setpoint());
      else if (BoilerProfile::is_mode_profile()) 
        doc["current_mode"] = String(this->internal_server->get_s_profile());
      else if (BoilerProfile::is_mode_water()) 
        doc["current_mode"] = String(this->internal_server->get_s_setpointwater());
      String send_json = "";
      serializeJson(doc, send_json);
      this->external_server->http_send_json(send_json);
      this->external_server->close_http_session();
      doc.clear();
  
      // проверяем есть ли новые настройки
      url_to_server = path_to_server + "/changed_by_client";
      this->external_server->start_http(url_to_server);
      response_code = this->external_server->http_get();
      if (response_code > 0){
        this->external_server->set_connected_to_server(true);
        deserializeJson(doc, this->external_server->http_get_string());
        this->external_server->close_http_session();
        // проверяем обновления по всем настройкам
        if (doc["settings"] == true){
          // есть изменения в режиме работы
          
          url_to_server = path_to_server + "/settings";
          this->external_server->start_http(url_to_server);
          int response_code = this->external_server->http_get();
            if (response_code > 0){
              DynamicJsonDocument sets(100);
              deserializeJson(sets,this->external_server->http_get_string());
              this->external_server->close_http_session();
              
              if (sets["mode"] == this->internal_server->get_s_setpoint()){
                // работаем по воздуху
                this->boiler_profile->set_boiler_mode(MODE_AIR);
              } else if (sets["mode"] == this->internal_server->get_s_setpointwater()){
                // работаем по теплоносителю
                this->boiler_profile->set_boiler_mode(MODE_WATER);
              } else if (sets["mode"] == this->internal_server->get_s_profile()){
                // работаем по термопрофилю
                this->boiler_profile->set_boiler_mode(MODE_PROFILE);
              }
              uint8_t need_temp = sets["target_temp"];
              if (!(need_temp >= WATER_TEMP_MIN && need_temp <= WATER_TEMP_MAX)) {
                need_temp = WATER_TEMP_MIN;
              }
              this->boiler_profile->set_target_temp(need_temp);
            } else {
              this->serial_error_report(url_to_server, response_code);
            }
        }
  
        if (doc["profile_weekdays"] == true){
          // есть изменения в профиле
          url_to_server = path_to_server + "/profile/weekdays";
          this->_external_profile_settings_init(url_to_server);
        }
  
        if (doc["profile_weekend"] == true){
          // есть изменения в профиле
          url_to_server = path_to_server + "/profile/weekend";
          this->_external_profile_settings_init(url_to_server);
        }
  
        if (doc["profile_custom"] == true){
          // есть изменения в профиле
          url_to_server = path_to_server + "/profile/custom";
          this->_external_profile_settings_init(url_to_server);
        }
  
        if (doc["profile_notfreeze"] == true){
          // есть изменения в профиле
          url_to_server = path_to_server + "/profile/notfreeze";
          this->_external_profile_settings_init(url_to_server);
        }
  
        if (doc["week"] == true){
          // есть изменения в графике на неделю
  
          url_to_server = path_to_server + "/week";
          this->external_server->start_http(url_to_server);
          int response_code = this->external_server->http_get();
          if (response_code > 0){
            DynamicJsonDocument sets(150);
            deserializeJson(sets, this->external_server->http_get_string());
            this->external_server->close_http_session();
  
            for (uint8_t i = 0; i < NUM_DAYS; i++){
              String num_of_day = "d";
              num_of_day += String(i);
              if (sets[num_of_day] == S_WEEKDAYS){
                this->boiler_profile->set_config_day_profile(i, PRESET_WEEKDAY);
              } else if (sets[num_of_day] == S_WEEKEND){
                this->boiler_profile->set_config_day_profile(i, PRESET_WEEKEND);
              } else if (sets[num_of_day] == S_CUSTOM){
                this->boiler_profile->set_config_day_profile(i, PRESET_CUSTOM);
              } else if (sets[num_of_day] == S_NOTFREEZE){
                this->boiler_profile->set_config_day_profile(i, PRESET_NOTFREEZE);
              }
            }
          } else {
            this->serial_error_report(url_to_server, response_code);
          }
        }
        
        BoilerProfile::save_configuration();
        
      } else {
        this->external_server->close_http_session();
        this->serial_error_report(url_to_server, response_code);
        this->external_server->set_connected_to_server(false);
      }
    }
  } 
}

void BoilerController::_external_profile_settings_init(String url) {
  this->external_server->start_http(url);
  int response_code = this->external_server->http_get();
  if (response_code > 0) {
    DynamicJsonDocument sets(150);
    deserializeJson(sets, this->external_server->http_get_string());
    this->external_server->close_http_session();

    for (uint8_t i = 0; i < NUM_PERIODS; i++) {
      String num_of_period = "h";
      num_of_period += String(i);
      uint8_t period_temp = sets[num_of_period];
      if (!(period_temp >= WATER_TEMP_MIN && period_temp <= WATER_TEMP_MAX)) {
        period_temp = WATER_TEMP_MIN;
    }
    this->boiler_profile->set_day_preset(PRESET_WEEKDAY, i, period_temp);
  }

  } else {
    this->serial_error_report(url, response_code);
  }
}

void BoilerController::serial_error_report(String target_url, int response_code) {
  Serial.print(F("path: "));
  Serial.println(target_url);
  Serial.print(F("responseCode: "));
  Serial.println(this->external_server->get_http_error(response_code));
}

void BoilerController::_fill_display_manager_configuration() {
  DisplayDataConfig display_data_config;
  display_data_config.is_wifi_connect = this->network_manager->is_wifi_connected();
  display_data_config.is_heating_on = this->relay_manager->is_heating_on();
  display_data_config.is_connected_to_server = this->external_server->get_connected_to_server();
  display_data_config.is_external_sensor = BoilerProfile::is_mode_air() || BoilerProfile::is_mode_profile();
  display_data_config.is_internal_sensor = BoilerProfile::is_mode_air();
  display_data_config.is_radio_connected = this->boiler_profile->is_radio_connected();
  display_data_config.is_overheat = ErrorService::is_set_error(ERROR_OVERHEAT) || ErrorService::is_set_error(ERROR_WATEROVERHEAT);
  display_data_config.is_pumpbroken = ErrorService::is_set_error(ERROR_PUMPBROKEN);
  display_data_config.is_ssrbroken = ErrorService::is_set_error(ERROR_SSRBROKEN);
  display_data_config.is_tempsensbroken = ErrorService::is_set_error(ERROR_TEMPSENSBROKEN);
  display_data_config.is_nopower = ErrorService::is_set_error(ERROR_NOPOWER);
  display_data_config.current_temperature = this->boiler_profile->get_current_temperature();
  display_data_config.target_temperature = BoilerProfile::get_target_temp();
  strncpy(display_data_config.current_day, this->boiler_profile->get_current_day("d/m/Y"), DISPLAY_CONF_STR_LENGTH);
  strncpy(display_data_config.current_time, this->boiler_profile->get_current_time("H:i"), DISPLAY_CONF_STR_LENGTH);
  this->display_manager->set_display_data_config(display_data_config);
}
