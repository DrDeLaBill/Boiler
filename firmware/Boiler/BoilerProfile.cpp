#include "BoilerProfile.h"

ClockRTC BoilerProfile::clock_rtc;
uint8_t BoilerProfile::session_target_temp_int = 0;
uint8_t BoilerProfile::session_boiler_mode = MODE_WATER;
BoilerConfiguration BoilerProfile::boiler_configuration;

BoilerProfile::BoilerProfile() {
  Serial.println(F("Boiler profile settings:"));
  BoilerProfile::start_eeprom();

  BoilerProfile::_serial_print_boiler_configuration();

  Serial.print(F("Boiler mode: "));
  Serial.println(String(BoilerProfile::boiler_configuration.boiler_mode));
  if (BoilerProfile::boiler_configuration.boiler_mode == 0xFF) {
    // EEPROM is empty
    BoilerProfile::set_default_settings();
  }
  Serial.print(F("boiler_name: "));
  Serial.println(BoilerProfile::boiler_configuration.boiler_name);
  Serial.print(F("boiler_id: "));
  Serial.println(String(BoilerProfile::boiler_configuration.boiler_id));

  BoilerProfile::session_target_temp_int = BoilerProfile::boiler_configuration.target_temp_int;
  BoilerProfile::session_boiler_mode = BoilerProfile::boiler_configuration.boiler_mode;
  
  if (BoilerProfile::is_set_session_boiler_mode(MODE_AIR) || BoilerProfile::is_set_session_boiler_mode(MODE_PROFILE)) {
    TemperatureSensor::set_radio_sensor(BoilerProfile::get_target_temp());
  }
  
  Serial.println(F("Boiler profile has been initialized."));
}

// Стирание eeprom
void BoilerProfile::clear_eeprom() {
  for (uint8_t i = 0; i < sizeof(BoilerProfile::boiler_configuration); i++) {
    EEPROM.put(i, 0xFF);
  }
  BoilerProfile::set_default_settings();
}

void BoilerProfile::set_default_settings(){
  // Задаем значения и настройки по умолчанию.
  Serial.println(F("EEPROM is empty. Loading default values."));
  BoilerProfile::boiler_configuration.boiler_mode = MODE_AIR;
  
  Serial.print(F("boiler_mode: "));
  Serial.println(BoilerProfile::boiler_configuration.boiler_mode);
  BoilerProfile::boiler_configuration.target_temp_int = TARGET_TEMP_INT_DEFAULT;
  BoilerProfile::boiler_configuration.target_temp_ext = TARGET_TEMP_EXT_DEFAULT;
  BoilerProfile::boiler_configuration.standby_flag = MODE_STANDBY;
  BoilerProfile::boiler_configuration.ssid[0] = '\0';
  BoilerProfile::boiler_configuration.password[0] = '\0';
  
  // с понедельника по пятницу рабочие дни, сб-вс - выходные.
  for (uint8_t i = 0; i < 5; i++) {
    BoilerProfile::boiler_configuration.profile[i] = PRESET_WEEKDAY;
  }
  BoilerProfile::boiler_configuration.profile[5] = PRESET_WEEKEND;
  BoilerProfile::boiler_configuration.profile[6] = PRESET_WEEKEND;

  // по умолчанию пока все температуры (в помещении) 20*С
  for (uint8_t i = 0; i < NUM_PRESETS; i++) {
    for (uint8_t j = 0; j < NUM_PERIODS; j++) {
      BoilerProfile::boiler_configuration.presets[i][j] = TARGET_TEMP_EXT_DEFAULT;
    }
  }

  memset(BoilerProfile::boiler_configuration.boiler_id, 0, ID_MAX_SIZE);
  
  String boiler_name_temp = "4d7920626f696c65722032"; //"My boiler 2"; "4d7920626f696c657220323333";
  boiler_name_temp.toCharArray(BoilerProfile::boiler_configuration.boiler_name, NAME_MAX_SIZE);
  Serial.print(F("boiler_name: "));
  Serial.println(boiler_name_temp);

  // Настройка текущей сессии
  BoilerProfile::session_target_temp_int = BoilerProfile::boiler_configuration.target_temp_int;
  BoilerProfile::session_boiler_mode = BoilerProfile::boiler_configuration.boiler_mode;

  // Сохранение настроек в EEPROM
  BoilerProfile::save_configuration();
}

void BoilerProfile::save_configuration() {
  Serial.println(F("SAVE NEW CONFIGURATION TO EEPROM:"));
  EEPROM.put(0, BoilerProfile::boiler_configuration);
  uint8_t *ptr = (uint8_t *)&BoilerProfile::boiler_configuration;
  for (uint8_t i = 0; i < sizeof(BoilerProfile::boiler_configuration); i++) {
    BoilerProfile::print_configuration_symbol(EEPROM.read(i));
//    EEPROM.put(i, ptr[i]);
  }
  Serial.println();
  BoilerProfile::session_boiler_mode = BoilerProfile::boiler_configuration.boiler_mode;
}

uint8_t BoilerProfile::get_target_temp(){
  // проверяем настройки - по воде, воздуху или термопрофилю
  if (BoilerProfile::session_boiler_mode == MODE_PROFILE){
    // котел работает по термопрофилю
    uint8_t num_preset = BoilerProfile::boiler_configuration.profile[ClockRTC::clock_get_day_of_week()];
    return BoilerProfile::boiler_configuration.presets[num_preset][BoilerProfile::period_of_day()];
  } else if (BoilerProfile::session_boiler_mode == MODE_AIR){
    // работаем по уставке по воздуху
    return BoilerProfile::boiler_configuration.target_temp_ext;
  } else if (BoilerProfile::session_boiler_mode == MODE_WATER){
    // работаем по уставке по воде
    return BoilerProfile::session_target_temp_int;
  }
}

void BoilerProfile::set_target_temp(uint8_t temp) {
  // сохранение новой установленной температуры
  // определяем, включен термопрофиль или уставка
  if (BoilerProfile::session_boiler_mode == MODE_PROFILE) {
    uint8_t num_preset = BoilerProfile::boiler_configuration.profile[ClockRTC::clock_get_day_of_week()];
    BoilerProfile::boiler_configuration.presets[num_preset][period_of_day()] = temp;
  } else if (BoilerProfile::session_boiler_mode == MODE_AIR){
    BoilerProfile::boiler_configuration.target_temp_ext = temp;
  } else if (BoilerProfile::session_boiler_mode == MODE_WATER){
    BoilerProfile::boiler_configuration.target_temp_int = temp;
    BoilerProfile::session_target_temp_int = temp;
  }
  BoilerProfile::save_configuration();
}

uint8_t BoilerProfile::period_of_day() {
  // вычисляем номер периода в сутках
  return ClockRTC::clock_get_hours() / 4;
}

void BoilerProfile::set_boiler_mode(uint8_t target_mode){
  // установка режима работы
  BoilerProfile::boiler_configuration.boiler_mode = target_mode;
  BoilerProfile::session_boiler_mode = target_mode;
  BoilerProfile::save_configuration();
}

void BoilerProfile::set_settings_standby(bool state_mode){
  BoilerProfile::boiler_configuration.standby_flag = state_mode;
  BoilerProfile::save_configuration();
}

void BoilerProfile::start_eeprom() {
  if (!EEPROM.begin(512)) {
    Serial.println(F("ERROR: Failed to initialise EEPROM"));
    Serial.println(F("Restarting..."));
    delay(200);
    ESP.restart();
  } else {
    Serial.println(F("EEPROM start"));
  }
}

void BoilerProfile::_serial_print_boiler_configuration() {
  // Read configuration from EEPROM
  uint8_t *ptr = (uint8_t *)&BoilerProfile::boiler_configuration;
  Serial.println(F("EEPROM settings:"));
  for (uint8_t i = 0; i < sizeof(BoilerProfile::boiler_configuration); i++) {
    BoilerProfile::print_configuration_symbol(EEPROM.read(i));
  }
  Serial.println();
}

void BoilerProfile::set_boiler_id(String boiler_id) {
  if (boiler_id == "") {
    Serial.println(F("Error set_boiler_id command"));
    return;
  }
  boiler_id.toCharArray(BoilerProfile::boiler_configuration.boiler_id, ID_MAX_SIZE);
}

void BoilerProfile::set_config_day_profile(uint8_t day, uint8_t value) {
   BoilerProfile::boiler_configuration.profile[day] = value;
}

String BoilerProfile::get_boiler_id() {
  return String(BoilerProfile::boiler_configuration.boiler_id);
}

String BoilerProfile::get_ssid() {
  return String(BoilerProfile::boiler_configuration.ssid);
}

String BoilerProfile::get_pass() {
  return String(BoilerProfile::boiler_configuration.password);
}

void BoilerProfile::set_wifi_settings(String ssid, String pass) {
  ssid.toCharArray(BoilerProfile::boiler_configuration.ssid, MAX_SIZE_SSID);
  pass.toCharArray(BoilerProfile::boiler_configuration.password, MAX_SIZE_PASS);
}

bool BoilerProfile::is_set_session_boiler_mode(ModeType search_mode) {
  return search_mode == BoilerProfile::session_boiler_mode;
}

bool BoilerProfile::is_set_config_boiler_mode(ModeType search_mode) {
  return search_mode == BoilerProfile::boiler_configuration.boiler_mode;
}

char *BoilerProfile::get_current_day(const char* fmt) {
  return ClockRTC::clock_get_time(fmt);
}

char *BoilerProfile::get_current_time(const char* fmt){
  return ClockRTC::clock_get_time(fmt);
}

BoilerConfiguration BoilerProfile::get_boiler_configuration() {
  return BoilerProfile::boiler_configuration;
}

void BoilerProfile::set_session_boiler_mode(uint8_t new_mode) {
  if (new_mode == MODE_AIR ||
      new_mode == MODE_PROFILE ||
      new_mode == MODE_WATER) {
    BoilerProfile::session_boiler_mode = new_mode;
  } else {
    BoilerProfile::session_boiler_mode = BoilerProfile::boiler_configuration.boiler_mode;
  }
}

uint8_t BoilerProfile::get_profile_for_week_day() {
  return BoilerProfile::boiler_configuration.profile[ClockRTC::clock_get_day_of_week()];
}

uint8_t BoilerProfile::get_session_boiler_mode() {
  return BoilerProfile::session_boiler_mode;
}

void BoilerProfile::set_day_preset(uint8_t day_number, uint8_t day_period, uint8_t value) {
    BoilerProfile::boiler_configuration.presets[day_number][day_period] = value;
}

void BoilerProfile::print_configuration_symbol(byte symbol) {
  if (0 <= symbol && symbol <= 31) {
    Serial.print(F("["));
    Serial.print(symbol);
    Serial.print(F("] "));
  } else {
    Serial.print(char(symbol));
  }
}

/*
   что здесь будет хранииться? Нам нужно понимать, если датчик температуры отваливается,
   то переходим на сохранение текущей температуры. Но при перезагрузке мы должны вернуться
   к основным настройкам. И снова проверить датчик.
   1 - если датчик пропал
*/
