#include "BoilerController.h"

ErrorService BoilerController::error_service;
NetworkManager BoilerController::network_manager;
DisplayManager BoilerController::display_manager;
TemperatureSensor BoilerController::temperature_sensor;
BoilerProfile BoilerController::boiler_profile;
EncoderManager BoilerController::encoder_manager;
RelayTemperature BoilerController::relay_manager;
PumpManager BoilerController::pump_manager;
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
  BoilerController::work_mode = MODE_STANDBY;
  NetworkManager::set_wifi_settings(
    BoilerProfile::get_ssid(),
    BoilerProfile::get_pass()
  );
  // проверяем, надо ли включаться или нет.
  if (BoilerProfile::boiler_configuration.standby_flag == MODE_STANDBY) {
    BoilerController::work_mode = MODE_WORK;
    Serial.println("WORK MODE");
    PumpManager::pump_on();
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
      TemperatureSensor::pid_regulating(BoilerProfile::is_set_session_boiler_mode(MODE_WATER), BoilerProfile::get_target_temp());
    }
    // нарисуем экран
    DisplayManager::fill_display_default_configuration();
    DisplayManager::paint();
    
    // измерим температуру
    TemperatureSensor::check_temperature();

    // проверим температуру ТТ реле.
    RelayTemperature::check_ssr_temp();
    // проверим энкодер
    if (EncoderManager::is_button_holded(BoilerController::work_mode)) {
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
    if (EncoderManager::is_button_holded(BoilerController::work_mode)) {
      // если было долгое нажатие кнопки - переходим в режим работы
      Serial.println(F("WORK MODE"));
      PumpManager::pump_on();
      DisplayManager::display_on();
      BoilerProfile::set_settings_standby(MODE_WORK);
      BoilerController::work_mode = MODE_WORK;
    }
    PumpManager::pump_off();
  }
  
  // Обработка ошибок
  ErrorService::init_error_actions();
}
