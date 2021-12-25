#include "BoilerController.h"

BoilerController::BoilerController() {
  this->controller_init();
}

void BoilerController::controller_init() {
  this->work_mode = MODE_STANDBY;
  this->p_mode = MODE_STANDBY;
  this->network_manager.set_wifi_settings(
    this->boiler_profile.get_ssid();
    this->boiler_profile.get_pass();
  );
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
    if (this->error_servise.is_set_error(ERROR_NOERROR)){
      this->boiler_profile.temperature_pid_regulating();
    } else {
      this->boiler_profile.temperature_pid_off();
    }
    // нарисуем экран
    //TODO: проверить правильность функции
    this->_fill_display_manager_configuration();
    this->display_manager.paint();
    // измерим температуру
    this->boiler_controller.check_temperature();
    // проверим температуру ТТ реле.
    // check_ssr_temp();
    // проверим энкодер
    if (this->encoder_manager.is_button_holded(this->work_mode)){
      // если было долгое нажатие кнопки - переходим в режим ожидания
      Serial.println(F("STANDBY MODE"));
      this->display_manager.display_off();
      this->boiler_profile.set_settings_standby(MODE_STANDBY);
      this->p_mode = MODE_STANDBY;
    }
    this->external_server.check_new_settings();
    // Поменялись ли ssid и password сети wifi
    this->_check_network_settings();
  }

  if (this->work_mode == MODE_STANDBY){
    // режим ожидания
    if (this->encoder_manager.is_button_holded(this->work_mode)){
      // если было долгое нажатие кнопки - переходим в режим работы
      Serial.println(F("WORK MODE"));
      this->pump_manager.pump_on();
      this->display_manager.display_on();
      set_settings_standby(WORK);
      this->work_mode = MODE_WORK;
      return;
    }
    this->display_manager.pump_off();
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

void BoilerController::_fill_display_manager_configuration() {
  DisplayDataConfig display_data_config;
  display_data_config.is_wifi_connect = this->network_manager.is_wifi_connected();
  display_data_config.is_heating_on = this->relay_manager.is_heating_on();
  display_data_config.is_connected_to_server = this->external_server.is_connected_to_server();
  display_data_config.is_external_sensor = this->boiler_profile.is_mode_air() || this->boiler_profile.is_mode_profile();
  display_data_config.is_internal_sensor = this->boiler_profile.is_mode_air();
  display_data_config.is_radio_connected = this->boiler_profile.is_radio_connected();
  display_data_config.is_overheat = this->error_service.is_set_error(ERROR_OVERHEAT) || this->error_service.is_set_error(ERROR_WATEROVERHEAT);
  display_data_config.is_pumpbroken = this->error_service.is_set_error(ERROR_PUMPBROKEN);
  display_data_config.is_ssrbroken = this->error_service.is_set_error(ERROR_SSRBROKEN);
  display_data_config.is_tempsensbroken = this->error_service.is_set_error(ERROR_TEMPSENSBROKEN);
  display_data_config.is_nopower = this->error_service.is_set_error(ERROR_NOPOWER);
  display_data_config.current_temperature = this->boiler_profile.get_current_temperature();
  display_data_config.target_temperature = this->boiler_profile.get_target_temp();
  strncpy(display_data_config.current_day, this->boiler_profile.get_current_day("d/m/Y"), sizeof(display_data_config.current_day));
  strncpy(display_data_config.current_time, this->boiler_profile.get_current_time("H:i"), sizeof(display_data_config.current_time));
  //TODO: вот до сюда
  this->display_manager.set_display_data_config(display_data_config);
}
