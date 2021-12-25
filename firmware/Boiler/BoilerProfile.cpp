#include "BoilerProfile.h"

BoilerProfile::BoilerProfile() {
  this->_start_eeprom();

  this->_serial_print_boiler_configuration();
  
  if (this->boiler_configuration.boiler_mode == 0xFF) {
    // EEPROM is empty
    this->set_default_settings();
    this->save_configuration();
  }
  Serial.print(F("\nboiler_name: "));
  Serial.println(this->boiler_name);
  Serial.print(F("boiler_id: "));
  Serial.println(String(this->boiler_id));

  this->user_target_temp_int = this->target_temp_int;
  this->user_boiler_mode = this->boiler_mode;
  
  //TODO: extern
  if (this->is_mode_air() || this->is_mode_profile()) {
    this->temperature_sensor.set_ext_sensor();
  }
}

// Стирание eeprom
void BoilerProfile::clear_eeprom () {
  this->_start_eeprom();

  for (uint8_t i = 0; i < sizeof(BoilerConfig); i++) {
    EEPROM.write(i, 0xFF);
  }
  EEPROM.commit();
  
  this->set_default_settings();
}

void BoilerProfile::set_default_settings(){
  // Задаем значения и настройки по умолчанию.
  Serial.println(F("EEPROM is empty. Loading default values."));
  this->boiler_configuration.boiler_mode = MODE_AIR;
  
  Serial.print(F("boiler_mode: "));
  Serial.println(this->boiler_mode);
  this->boiler_configuration.target_temp_int = TARGET_TEMP_INT_DEFAULT;
  this->boiler_configuration.target_temp_ext = TARGET_TEMP_EXT_DEFAULT;
  this->boiler_configuration.standby_flag = WORK;
  this->boiler_configuration.ssid[0] = '\0';
  this->boiler_configuration.password[0] = '\0';
  
  // с понедельника по пятницу рабочие дни, сб-вс - выходные.
  for (uint8_t i = 0; i < 5; i++) {
    this->boiler_configuration.profile[i] = PRESET_WEEKDAY;
  }
  this->boiler_configuration.profile[5] = PRESET_WEEKEND;
  this->boiler_configuration.profile[6] = PRESET_WEEKEND;

  // по умолчанию пока все температуры (в помещении) 20*С
  for (uint8_t i = 0; i < NUM_PRESETS; i++) {
    for (uint8_t j = 0; j < NUM_PERIODS; j++) {
      this->boiler_configuration.presets[i][j] = TARGET_TEMP_EXT_DEFAULT;
    }
  }

  memset(this->boiler_configuration.boiler_id, 0, ID_MAX_SIZE);
  
  String boiler_name_temp = "My boiler 2";
  boiler_name_temp.toCharArray(this->boiler_configuration.boiler_name, NAME_MAX_SIZE);
  Serial.print(F("boiler_name: "));
  Serial.println(boiler_name_temp);

  // Настройка текущей сессии
  this->user_target_temp_int = BoilerCfg.target_temp_int;
  this->user_boiler_mode = BoilerCfg.boiler_mode;

  // Сохранение настроек в EEPROM
  this->save_configuration();
}

void BoilerProfile::save_configuration() {
  uint8_t *ptr = (uint8_t *)this->&boiler_configuration;
  bool all_correct = true;
  for (uint8_t i = 0; i < sizeof(this->boiler_configuration) && all_correct; i++) {
    if (ptr[i] != EEPROM.read(i)) {
      all_correct = false;
    }
  }

  if (all_correct == false) {
    ptr = (uint8_t *)this->&boiler_configuration;
    for (uint8_t i = 0; i < sizeof(this->boiler_configuration); i++) {
      EEPROM.write(i, ptr[i]);
    }
    EEPROM.commit();
    this->user_boiler_mode = this->boiler_configuration.boiler_mode;
  }
}

uint8_t BoilerProfile::get_target_temp(){
  // проверяем настройки - по воде, воздуху или термопрофилю
  if (this->user_boiler_mode == MODE_PROFILE){
    // котел работает по термопрофилю
    uint8_t num_preset = this->boiler_configuration.profile[clock_get_day_of_week()];
    return this->boiler_configuration.presets[num_preset][period_of_day()];
  } else if (this->user_boiler_mode == MODE_AIR){
    // работаем по уставке по воздуху
    return this->boiler_configuration.target_temp_ext;
  } else if (this->user_boiler_mode == MODE_WATER){
    // работаем по уставке по воде
    return this->user_target_temp_int;
  }
}

void BoilerProfile::set_target_temp(uint8_t temp) {
  // сохранение новой установленной температуры
  // определяем, включен термопрофиль или уставка
  if (this->user_boiler_mode == MODE_PROFILE) {
    uint8_t num_preset = this->boiler_configuration.profile[clock_get_day_of_week()];
    this->boiler_configuration.presets[num_preset][period_of_day()] = temp;
  } else if (this->user_boiler_mode == MODE_AIR){
    this->boiler_configuration.target_temp_ext = temp;
  } else if (this->user_boiler_mode == MODE_WATER){
    this->boiler_configuration.target_temp_int = temp;
    this->user_target_temp_int = temp;
  }
  this->save_configuration();
}

uint8_t BoilerProfile::period_of_day() {
  // вычисляем номер периода в сутках
  return this->clock_rtc.clock_get_hours() / 4;
}

void BoilerProfile::set_boiler_mode(uint8_t target_mode){
  // установка режима работы
  this->boiler_configuration.boiler_mode = target_mode;
  this->user_boiler_mode = target_mode;
  //TODO: extern
//  if (this->user_boiler_mode == MODE_AIR || this->user_boiler_mode == MODE_PROFILE){
//    radio_init(); //(set_ext_sensor)
//  }
  this->save_configuration();
}

void BoilerProfile::set_settings_standby(bool state){
  this->boiler_configuration.standby_flag = state;
  this->save_configuration();
}

void BoilerProfile::_start_eeprom() {
  if (!EEPROM.begin(512)) {
    Serial.println(F("ERROR: Failed to initialise EEPROM"));
    Serial.println(F("Restarting..."));
    delay(200);
    ESP.restart();
  }
}

void BoilerProfile::_serial_print_boiler_configuration() {
  // Read configuration from EEPROM
  uint8_t *ptr = (uint8_t *)this->&boiler_configuration;
  for (uint8_t i = 0; i < sizeof(this->boiler_configuration); i++) {
    ptr[i] = EEPROM.read(i);
    Serial.print(ptr[i]);
    Serial.print(F(" "));
  }
}

void BoilerProfile::set_boiler_id(String boiler_id) {
  if (boiler_id == "") {
    Serial.println(F("Error set_boiler_id command"));
    return;
  }
  boiler_id.toCharArray(this->boiler_configuration.boiler_id, ID_MAX_SIZE);
}

String BoilerProfile::get_boiler_id() {
  return String(this->boiler_configuration.boiler_id);
}

String BoilerProfile::get_ssid() {
  return String(this->boiler_configuration.ssid);
}

String BoilerProfile::get_pass() {
  return String(this->boiler_configuration.pass);
}

void BoilerProfile::set_wifi_settings(String ssid, String pass) {
  ssid.toCharArray(this->boiler_configuration.ssid, MAX_SIZE_SSID);
  pass.toCharArray(this->boiler_configuration.password, MAX_SIZE_PASS);
}

bool is_mode_air() {
    return this->user_boiler_mode == MODE_AIR;
}

bool is_mode_water() {
    return this->user_boiler_mode == MODE_WATER;
}

bool is_mode_profile() {
    return this->user_boiler_mode == MODE_PROFILE;
}

void temperature_pid_regulating() {
  this->temperature_sensor.pid_regulating();
}

void temperature_pid_off() {
  this->temperature_sensor.pid_off();
}

bool is_radio_connected() {
  return this->temperature_sensor.is_radio_connected();
}

uint8_t get_current_temperature() {
  return this->temperature_sensor.get_current_temperature();
}

char *get_current_day(const char* fmt) {
  return this->clock_get_time(fmt);
}

char *get_current_time(const char* fmt){
  return this->clock_get_time(fmt);
}

void check_temperature() {
  this->temperature_sensor.check_temp();
}
/*
   что здесь будет хранииться? Нам нужно понимать, если датчик температуры отваливается,
   то переходим на сохранение текущей температуры. Но при перезагрузке мы должны вернуться
   к основным настройкам. И снова проверить датчик.
   1 - если датчик пропал
*/
