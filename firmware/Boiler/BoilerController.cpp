#include "BoilerController.h"

void BoilerController::controller_init() {
  this->work_mode = MODE_STANDBY;
}

void BoilerController::controller_run() {
  // Проверка наличия команд через Serial порт
  this->_check_serial_port_commands();

  if (this->work_mode == MODE_WORK){
    // Проверка наличия ошибок
    this->error_service.check_failure();
    uint8_t errors[ERRORS_COUNT] = {};
    this->error_service.get_errors_list(errors);
    

    // проверим нагрев
    if (this->error_servise.is_set_error(NOERROR)){
      pid_regulating();
    } else {
      pid_off();
    }
    
    // нарисуем экран
    paint();
  
    // измерим температуру
    check_temp();
  
    // проверим температуру ТТ реле.
    // check_ssr_temp();
  
    // проверим энкодер
    if (check_encoder(this->work_mode) == BUTTON_HOLDED){
      // если было долгое нажатие кнопки - переходим в режим ожидания
      Serial.println(F("STANDBY MODE"));
      display_off();
      set_settings_standby(STANDBY);
      p_mode = MODE_STANDBY;
    }

    check_new_settings();
    // Поменялись ли ssid и password сети wifi
    this->_check_network_settings();
  }

  if (this->work_mode == MODE_STANDBY){
    // режим ожидания
    if (check_encoder(this->work_mode) == BUTTON_HOLDED){
      // если было долгое нажатие кнопки - переходим в режим работы
      Serial.println(F("WORK MODE"));
      pump_on();
      display_on();
      set_settings_standby(WORK);
      this->work_mode = MODE_WORK;
      return;
    }
    
    pump_off();
  }
}

void BoilerController::_check_serial_port_commands() {
  this->command_manager.check_commands();
  if (this->command_manager.get_command() == "set_boiler_id") {
    String new_boiler_id = this->command_manager.get_new_boiler_id();
    this->boiler_profile.set_boiler_id(new_boiler_id);
  } else if (this->command_manager.get_command() == "get_boiler_id") {
    String boiler_id = this->boiler_profile.get_boiler_id();
    Serial.print("Boiler ID: ");
    Serial.println(boiler_id);
  }
}

// Совпадают ли ssid и pass BoilerProfile и NetworkManager
void BoilerController::_check_network_settings() {
  if (this->network_manager.get_ssid() != this->boiler_configuration.get_ssid() || this->network_manager.get_pass() != this->boiler_configuration.get_pass()) {
    this->boiler_configuration.set_wifi_settings(
      this->network_manager.get_ssid(),
      this->network_manager.get_pass()
    );
  }
}
