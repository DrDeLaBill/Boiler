#include "BoilerController.h"

CommandManager BoilerController::command_manager;

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
  this->temperature_sensor = new TemperatureSensor();
  this->boiler_profile = new BoilerProfile();
  this->internal_server = new InternalServer();
  this->encoder_manager = new EncoderManager();
  this->relay_manager = new RelayTemperature();
  this->pump_manager = new PumpManager();

  this->work_mode = MODE_STANDBY;
  this->network_manager->set_wifi_settings(
    this->boiler_profile->get_ssid(),
    this->boiler_profile->get_pass()
  );
  // проверяем, надо ли включаться или нет.
  if (BoilerProfile::boiler_configuration.standby_flag == MODE_STANDBY) {
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
  CommandManager::check_commands();

  if (this->work_mode == MODE_WORK) {
    // Проверка наличия ошибок
    this->error_service->check_failure();
    uint8_t errors[ERRORS_COUNT] = {};
    this->error_service->get_errors_list(errors);
    // проверим нагрев
    if (ErrorService::is_set_error(ERROR_NOERROR)) {
      this->temperature_sensor->pid_off();
    } else {
      this->temperature_sensor->pid_regulating(BoilerProfile::is_mode_water(), BoilerProfile::get_target_temp());
    }
    // нарисуем экран
    //TODO: проверить правильность функции
    this->_fill_display_manager_configuration();
    this->display_manager->paint();
    
    // измерим температуру
    this->temperature_sensor->check_temperature();

    // проверим температуру ТТ реле.
    this->relay_manager->check_ssr_temp();
    // проверим энкодер
    if (this->encoder_manager->is_button_holded(this->work_mode)) {
      // если было долгое нажатие кнопки - переходим в режим ожидания
      Serial.println(F("STANDBY MODE"));
      this->display_manager->display_off();
      this->boiler_profile->set_settings_standby(MODE_STANDBY);
      this->work_mode = MODE_STANDBY;
    }
    ExternalServer::check_settings();
    // Поменялись ли ssid и password сети wifi
    NetworkManager::check_new_settings();
  } else if (this->work_mode == MODE_STANDBY) {
    // режим ожидания
    if (this->encoder_manager->is_button_holded(this->work_mode)) {
      // если было долгое нажатие кнопки - переходим в режим работы
      Serial.println(F("WORK MODE"));
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

void BoilerController::_fill_display_manager_configuration() {
  DisplayManager::display_data_config.is_wifi_connect = this->network_manager->is_wifi_connected();
  DisplayManager::display_data_config.is_heating_on = this->relay_manager->is_heating_on();
  DisplayManager::display_data_config.is_connected_to_server = this->external_server->get_connected_to_server();
  DisplayManager::display_data_config.is_external_sensor = BoilerProfile::is_mode_air() || BoilerProfile::is_mode_profile();
  DisplayManager::display_data_config.is_internal_sensor = BoilerProfile::is_mode_air();
  DisplayManager::display_data_config.is_radio_connected = this->temperature_sensor->is_radio_connected();
  DisplayManager::display_data_config.is_overheat = ErrorService::is_set_error(ERROR_OVERHEAT) || ErrorService::is_set_error(ERROR_WATEROVERHEAT);
  DisplayManager::display_data_config.is_pumpbroken = ErrorService::is_set_error(ERROR_PUMPBROKEN);
  DisplayManager::display_data_config.is_ssrbroken = ErrorService::is_set_error(ERROR_SSRBROKEN);
  DisplayManager::display_data_config.is_tempsensbroken = ErrorService::is_set_error(ERROR_TEMPSENSBROKEN);
  DisplayManager::display_data_config.is_nopower = ErrorService::is_set_error(ERROR_NOPOWER);
  DisplayManager::display_data_config.current_temperature = TemperatureSensor::get_current_temperature();
  DisplayManager::display_data_config.target_temperature = BoilerProfile::get_target_temp();
  strncpy(DisplayManager::display_data_config.current_day, this->boiler_profile->get_current_day("d/m/Y"), DISPLAY_CONF_STR_LENGTH);
  strncpy(DisplayManager::display_data_config.current_time, this->boiler_profile->get_current_time("H:i"), DISPLAY_CONF_STR_LENGTH);
}
