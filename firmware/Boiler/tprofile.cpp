/*

   Чтение, запись термопрофиля в eeprom.
   Получение текущей уставки температуры.
   Получение текущих даты и времени.

*/

#include "tprofile.h"

BoilerConfig BoilerCfg;

uint8_t user_target_temp_int = 0;   // требуемая температура теплоносителя (в данной сессии)
uint8_t user_boiler_mode = 0;       // режим работы котла (в данной сессии)

extern String current_ssid;
extern String current_password;

/*
   что здесь будет хранииться? Нам нужно понимать, если датчик температуры отваливается,
   то переходим на сохранение текущей температуры. Но при перезагрузке мы должны вернуться
   к основным настройкам. И снова проверить датчик.
   1 - если датчик пропал
*/


// Стирание eeprom
void clearEeprom () {
  if (!EEPROM.begin(512)) {
    Serial.println("Failed to initialise EEPROM");
    Serial.println("Restarting...");
    delay(1000);
    ESP.restart();
  }

  for (uint8_t i = 0; i < sizeof(BoilerConfig); i++) {
    EEPROM.write(i, 0xFF);
  }
  EEPROM.commit();
  
  defaultSettings();
  save_cfg();
  
//  EEPROM.commit();
//  ESP.restart();
}

void defaultSettings(){
  // Задаем значения и настройки по умолчанию.

  Serial.println("EEPROM is empty. Loading default values.");
  BoilerCfg.boiler_mode = MODE_AIR;
  Serial.print("boiler_mode: ");
  Serial.println(BoilerCfg.boiler_mode);
  BoilerCfg.target_temp_int = TARGET_TEMP_INT_DEFAULT;
  BoilerCfg.target_temp_ext = TARGET_TEMP_EXT_DEFAULT;
  BoilerCfg.standby_flag = WORK;
  BoilerCfg.ssid[0] = '\0';
  BoilerCfg.password[0] = '\0';
  // с понедельника по пятницу рабочие дни, сб-вс - выходные.
  for (uint8_t i = 0; i < 5; i++) {
    BoilerCfg.profile[i] = PRESET_WEEKDAY;
  }
  BoilerCfg.profile[5] = PRESET_WEEKEND;
  BoilerCfg.profile[6] = PRESET_WEEKEND;

  // по умолчанию пока все температуры (в помещении) 20*С
  for (uint8_t i = 0; i < NUM_PRESETS; i++) {
    for (uint8_t j = 0; j < NUM_PERIODS; j++) {
      BoilerCfg.presets[i][j] = TARGET_TEMP_EXT_DEFAULT;
    }
  }

  memset(BoilerCfg.boiler_id, 0, ID_MAX_SIZE);
  String boiler_name_temp = "My boiler 2";
  boiler_name_temp.toCharArray(BoilerCfg.boiler_name, NAME_MAX_SIZE);
  Serial.print("boiler_name: ");
  Serial.println(String(BoilerCfg.boiler_name));
  user_target_temp_int = BoilerCfg.target_temp_int;
  user_boiler_mode = BoilerCfg.boiler_mode;
  current_ssid = String(BoilerCfg.ssid);
  current_password = String(BoilerCfg.password);
}

void cfg_init() {
  if (!EEPROM.begin(512)) {
    Serial.println("Failed to initialise EEPROM");
    Serial.println("Restarting...");
    delay(1000);
    ESP.restart();
  }

  // Read cfg from EEPROM
  uint8_t *ptr = (uint8_t *)&BoilerCfg;
  for (uint8_t i = 0; i < sizeof(BoilerConfig); i++) {
    ptr[i] = EEPROM.read(i);
    Serial.print(ptr[i]);
    Serial.print(" ");
  }

  if (BoilerCfg.boiler_mode == 0xFF) {
    // EEPROM is empty
    defaultSettings();
    save_cfg();
  }
  Serial.println("");
  Serial.print("boiler_name: ");
  Serial.println(BoilerCfg.boiler_name);
  Serial.print("boiler_id: ");
  Serial.println(String(BoilerCfg.boiler_id));

  user_target_temp_int = BoilerCfg.target_temp_int;
  user_boiler_mode = BoilerCfg.boiler_mode;
  if (user_boiler_mode == MODE_AIR || user_boiler_mode == MODE_PROFILE) {
    set_ext_sensor();
  }
  current_ssid = String(BoilerCfg.ssid);
  current_password = String(BoilerCfg.password);
}

void save_cfg() {
  uint8_t *ptr = (uint8_t *)&BoilerCfg;
  bool all_correct = true;
  for (uint8_t i = 0; i < sizeof(BoilerConfig); i++) {
    if (ptr[i] != EEPROM.read(i)) {
      all_correct = false;
      break;
    }
  }
  if (current_ssid != String(BoilerCfg.ssid) || current_password != String(BoilerCfg.password)) {
    all_correct = false;
    current_ssid.toCharArray(BoilerCfg.ssid, MAX_SIZE_SSID);
    current_password.toCharArray(BoilerCfg.password, MAX_SIZE_PASS);
  }

  if (all_correct == false) {
    ptr = (uint8_t *)&BoilerCfg;
    for (uint8_t i = 0; i < sizeof(BoilerConfig); i++) {
      EEPROM.write(i, ptr[i]);
    }
    EEPROM.commit();
    user_boiler_mode = BoilerCfg.boiler_mode;
  }
}

uint8_t get_target_temp(){
  // проверяем настройки - по воде, воздуху или термопрофилю
  if (user_boiler_mode == MODE_PROFILE){
    // котел работает по термопрофилю
    
    uint8_t num_preset = BoilerCfg.profile[clock_get_day_of_week()];
    return BoilerCfg.presets[num_preset][period_of_day()];
  } else if (user_boiler_mode == MODE_AIR){
    // работаем по уставке по воздуху
      
    return BoilerCfg.target_temp_ext;
  } else if (user_boiler_mode == MODE_WATER){
    // работаем по уставке по воде
     
    return user_target_temp_int;
  }
}

void set_target_temp(uint8_t temp) {
  // сохранение новой установленной температуры
  // определяем, включен термопрофиль или уставка

  if (user_boiler_mode == MODE_PROFILE) {
    uint8_t num_preset = BoilerCfg.profile[clock_get_day_of_week()];
    BoilerCfg.presets[num_preset][period_of_day()] = temp;
  } else if (user_boiler_mode == MODE_AIR){
    BoilerCfg.target_temp_ext = temp;
  } else if (user_boiler_mode == MODE_WATER){
    BoilerCfg.target_temp_int = temp;
    user_target_temp_int = temp;
  }
  save_cfg();
}

uint8_t period_of_day() {
  // вычисляем номер периода в сутках
  return clock_get_hours() / 4;
}

void set_boiler_mode(uint8_t target_mode){
  // установка режима работы
  
  BoilerCfg.boiler_mode = target_mode;
  user_boiler_mode = target_mode;
  if (user_boiler_mode == MODE_AIR || user_boiler_mode == MODE_PROFILE){
    set_ext_sensor();
  }
  save_cfg();
}

void set_settings_standby(bool state){
  BoilerCfg.standby_flag = state;
  save_cfg();
}
