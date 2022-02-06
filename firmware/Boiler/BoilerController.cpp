#include "BoilerController.h"

ErrorService BoilerController::error_service;
NetworkManager BoilerController::network_manager;
DisplayManager BoilerController::display_manager;
TemperatureSensor BoilerController::temperature_sensor;
BoilerProfile BoilerController::boiler_profile;
InternalServer BoilerController::internal_server;

CommandManager BoilerController::command_manager;
bool BoilerController::work_mode;

BoilerController::BoilerController() {
  Serial.println(F("\n######################################################"));
  Serial.println(F("Initialization boiler started."));
  if(!SPIFFS.begin(true)){
    // "При монтировании SPIFFS произошла ошибка"
    Serial.println(F("An Error has occurred while mounting SPIFFS"));
    ESP.restart();
  }
  this->encoder_manager = new EncoderManager();
  this->relay_manager = new RelayTemperature();
  this->pump_manager = new PumpManager();

  BoilerController::work_mode = MODE_STANDBY;
  NetworkManager::set_wifi_settings(
    BoilerProfile::get_ssid(),
    BoilerProfile::get_pass()
  );
  // проверяем, надо ли включаться или нет.
  if (BoilerProfile::boiler_configuration.standby_flag == MODE_STANDBY) {
    BoilerController::work_mode = MODE_WORK;
    Serial.println("WORK MODE");
    this->pump_manager->pump_on();
  } else {
    BoilerController::work_mode = MODE_STANDBY;
    Serial.println("STANDBY MODE");
    DisplayManager::display_off();
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

  if (BoilerController::work_mode == MODE_WORK) {
    // Проверка наличия ошибок
    ErrorService::check_failure();
    uint8_t errors[ERRORS_COUNT] = {};
    ErrorService::get_errors_list(errors);
    // проверим нагрев
    if (ErrorService::is_set_error(ERROR_NOERROR)) {
      TemperatureSensor::pid_off();
    } else {
      TemperatureSensor::pid_regulating(BoilerProfile::is_mode_water(), BoilerProfile::get_target_temp());
    }
    // нарисуем экран
    //TODO: проверить правильность функции
    this->_fill_display_manager_configuration();
    DisplayManager::paint();
    
    // измерим температуру
    TemperatureSensor::check_temperature();

    // проверим температуру ТТ реле.
    this->relay_manager->check_ssr_temp();
    // проверим энкодер
    if (this->encoder_manager->is_button_holded(BoilerController::work_mode)) {
      // если было долгое нажатие кнопки - переходим в режим ожидания
      Serial.println(F("STANDBY MODE"));
      DisplayManager::display_off();
      BoilerProfile::set_settings_standby(MODE_STANDBY);
      BoilerController::work_mode = MODE_STANDBY;
    }
    ExternalServer::check_settings();
    // Поменялись ли ssid и password сети wifi
    NetworkManager::check_new_settings();
  } else if (BoilerController::work_mode == MODE_STANDBY) {
    // режим ожидания
    if (this->encoder_manager->is_button_holded(BoilerController::work_mode)) {
      // если было долгое нажатие кнопки - переходим в режим работы
      Serial.println(F("WORK MODE"));
      this->pump_manager->pump_on();
      DisplayManager::display_on();
      BoilerProfile::set_settings_standby(MODE_WORK);
      BoilerController::work_mode = MODE_WORK;
    }
    this->pump_manager->pump_off();
  }
  
  // Обработка ошибок
  ErrorService::init_error_actions();
}

//TODO: для чего и откуда
//bool BoilerController::_button_holded_action() {
//  uint8_t button_state = this->encoder_manager->check_encoder(BoilerController::work_mode);
//  DisplayManager::rotary_encoder_action(
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

//TODO: переместить в DisplayManager
void BoilerController::_fill_display_manager_configuration() {
  DisplayManager::display_data_config.is_wifi_connect = NetworkManager::is_wifi_connected();
  DisplayManager::display_data_config.is_heating_on = this->relay_manager->is_heating_on();
  DisplayManager::display_data_config.is_connected_to_server = ExternalServer::get_connected_to_server();
  DisplayManager::display_data_config.is_external_sensor = BoilerProfile::is_mode_air() || BoilerProfile::is_mode_profile();
  DisplayManager::display_data_config.is_internal_sensor = BoilerProfile::is_mode_air();
  DisplayManager::display_data_config.is_radio_connected = TemperatureSensor::is_radio_connected();
  DisplayManager::display_data_config.is_overheat = ErrorService::is_set_error(ERROR_OVERHEAT) || ErrorService::is_set_error(ERROR_WATEROVERHEAT);
  DisplayManager::display_data_config.is_pumpbroken = ErrorService::is_set_error(ERROR_PUMPBROKEN);
  DisplayManager::display_data_config.is_ssrbroken = ErrorService::is_set_error(ERROR_SSRBROKEN);
  DisplayManager::display_data_config.is_tempsensbroken = ErrorService::is_set_error(ERROR_TEMPSENSBROKEN);
  DisplayManager::display_data_config.is_nopower = ErrorService::is_set_error(ERROR_NOPOWER);
  DisplayManager::display_data_config.current_temperature = TemperatureSensor::get_current_temperature();
  DisplayManager::display_data_config.target_temperature = BoilerProfile::get_target_temp();
  strncpy(DisplayManager::display_data_config.current_day, BoilerProfile::get_current_day("d/m/Y"), DISPLAY_CONF_STR_LENGTH);
  strncpy(DisplayManager::display_data_config.current_time, BoilerProfile::get_current_time("H:i"), DISPLAY_CONF_STR_LENGTH);
}
