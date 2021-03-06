#include "TemperatureSensor.h"

uint8_t TemperatureSensor::current_temp = 0;
uint8_t TemperatureSensor::radio_connected = RADIO_WAIT;
uint8_t TemperatureSensor::sens_temp_tries = 5;
uint32_t TemperatureSensor::ds18b20_last_time = 0;
GyverPID TemperatureSensor::regulator_AIR(kP_air, kI_air, kD_air, dT);
GyverPID TemperatureSensor::regulator_WATER(kP_water, kI_water, kD_water, dT);
//OneWire TemperatureSensor::oneWire(ONE_WIRE_BUS);
DallasTemperature TemperatureSensor::sensors((new OneWire(ONE_WIRE_BUS)));
float TemperatureSensor::current_temp_water = 0;                   
float TemperatureSensor::current_temp_air = 0;                    
uint32_t TemperatureSensor::pid_last_time = 0;              
uint32_t TemperatureSensor::pwm_set_0_time = 0;
uint32_t TemperatureSensor::check_ssr_last_time = 0;       
uint8_t TemperatureSensor::check_ssr_last_temp = 0;
float TemperatureSensor::kP_air = 7;
float TemperatureSensor::kI_air = 0.5;
float TemperatureSensor::kD_air = 0.3;
float TemperatureSensor::kP_water = 7;
float TemperatureSensor::kI_water = 0.5;
float TemperatureSensor::kD_water = 0.3;
float TemperatureSensor::dT = 1000;
const uint32_t TemperatureSensor::period_msec = 1000;
bool TemperatureSensor::is_heating_on = false;

TemperatureSensor::TemperatureSensor() {
  TemperatureSensor::temp_init();
  TemperatureSensor::pid_init();
  Serial.print(F("Current temp water: "));
  Serial.println(TemperatureSensor::current_temp_water);
  Serial.println(F("Temperature sensor init"));
}

void TemperatureSensor::temp_init() {
  TemperatureSensor::sensors.begin();
  TemperatureSensor::sensors.setWaitForConversion(false);
  TemperatureSensor::sensors.requestTemperatures();
}

void TemperatureSensor::check_temperature() {
  uint8_t sens_status = NO_TEMP;
  // считывание данных с датчика температуры с некоторой периодичностью
  if (millis() - TemperatureSensor::ds18b20_last_time >= DS18B20_MEAS_PERIOD){
    TemperatureSensor::ds18b20_last_time = millis();
    sens_status = TemperatureSensor::update_current_temp_water();
  }

  if (sens_status == GOT_TEMP){
    if (TemperatureSensor::current_temp_water >= WATER_TEMP_LIM){
      // если температура теплоносителя стала аварийно высокой
      // здесь надо отключать силовое питание!
      Serial.print(F("Water overheat: "));
      Serial.print(TemperatureSensor::current_temp_water);
      Serial.println(F(" *C"));
      ErrorService::add_error(ERROR_WATEROVERHEAT);
      DisplayManager::set_page_name(pageError);
    } else {
      // если температура понизилась, то может и не стоит возвращаться в обычный режим?
      ErrorService::remove_error(ERROR_WATEROVERHEAT);
    }
      
    if (ErrorService::is_set_error(ERROR_TEMPSENSBROKEN)){
      // если датчик температуры был неисправен, а теперь починился
      ErrorService::remove_error(ERROR_TEMPSENSBROKEN);
    }

    if (BoilerProfile::session_boiler_mode == MODE_WATER){
      TemperatureSensor::set_current_temp_like_water_temp();
    }
  }

  if (sens_status == TEMP_SENS_ERROR){
    // датчик не выходит на связь
    ErrorService::add_error(ERROR_TEMPSENSBROKEN);
  }
  
  sens_status = TemperatureSensor::update_radio_temp();

  if (sens_status == GOT_EXT_TEMP){
    if (TemperatureSensor::is_radio_lost()){
      // если датчик отваливался, а теперь появился
      // проверим, надо ли нам переключить режим обратно
      if (BoilerProfile::is_set_config_boiler_mode(MODE_AIR) || BoilerProfile::is_set_config_boiler_mode(MODE_PROFILE)){
        BoilerProfile::session_boiler_mode = BoilerProfile::boiler_configuration.boiler_mode;
        ErrorService::remove_error(ERROR_RADIOSENSOR);
      }
    }
    TemperatureSensor::set_radio_on();
    
    if (BoilerProfile::is_set_session_boiler_mode(MODE_AIR) || BoilerProfile::is_set_session_boiler_mode(MODE_PROFILE)){
      TemperatureSensor::set_current_temp_like_air_temp();
    }
  } else if (sens_status == RADIO_ERROR){
    // датчика нет
    Serial.println("RADIO_ERROR");
    if (TemperatureSensor::is_radio_on() || TemperatureSensor::is_radio_wait()){
      // а до этого был или должен был быть
      // то переключаем режим работы на уставку по воде
      Serial.println("validate true");
      if (BoilerProfile::is_set_config_boiler_mode(MODE_AIR) || BoilerProfile::is_set_config_boiler_mode(MODE_PROFILE)){
        Serial.println("set boiler mode water");
        BoilerProfile::session_boiler_mode = MODE_WATER;
        BoilerProfile::session_target_temp_int = (uint8_t)TemperatureSensor::get_current_temp_water();
        TemperatureSensor::set_current_temp_like_water_temp();
      }
    }
    TemperatureSensor::set_radio_lost();
  }
}

void TemperatureSensor::set_radio_sensor(uint8_t target_temperature){
  RadioSensor::clear_timeout_radio_sens();
  if (TemperatureSensor::radio_connected != RADIO_ON) {
    TemperatureSensor::radio_connected = RADIO_WAIT;
  }
  TemperatureSensor::current_temp = target_temperature;          // пока датчик не отправил данные, загружаем это значение
}

void TemperatureSensor::pid_regulating(bool is_mode_water, uint8_t target_temperature){
  if ((millis() > pwm_set_0_time) && digitalRead(SSR1_OUT_PIN)){
    TemperatureSensor::pid_off();
  }

  // отправка показаний с датчиков на пид регулятор с определенной периодичностью
  if (millis() - TemperatureSensor::pid_last_time > 1000){
    TemperatureSensor::pid_last_time = millis();

    // разные пиды для режимов работы по воздуху и теплоносителю
    if (is_mode_water && (ErrorService::is_no_errors() || ErrorService::if_single_error(ERROR_RADIOSENSOR))) {       // работать по воде|
      TemperatureSensor::TemperatureSensor::regulator_WATER.setpoint = target_temperature;                           // Сообщаем регулятору температуру к которой следует стремиться
      TemperatureSensor::TemperatureSensor::regulator_WATER.input = TemperatureSensor::current_temp;                 // Сообщаем регулятору текущую температуру к которой будем стремиться
      
      TemperatureSensor::pwm(TemperatureSensor::TemperatureSensor::regulator_WATER.getResultTimer());                // включаем ТТР, опираясь на температуру воды

      TemperatureSensor::TemperatureSensor::regulator_AIR.integral = 0;                                              //интегральная составляющая для воздуха не должна рости

      if (abs(target_temperature - TemperatureSensor::current_temp) > SCATTER_TEMP){                                 // если текущая температура не достигла диапазона регулирования, недопускаем накопление интегральной ошибки
        TemperatureSensor::TemperatureSensor::regulator_WATER.integral = 0;
      }
    } else if (ErrorService::is_no_errors() && RadioSensor::is_sensor_online()) {
      // если сейчас работаем по воздуху или термопрофилю
      TemperatureSensor::TemperatureSensor::regulator_AIR.setpoint = target_temperature;                             // Сообщаем регулятору температуру к которой следует стремиться
      TemperatureSensor::TemperatureSensor::regulator_AIR.input = TemperatureSensor::current_temp;                   // Сообщаем регулятору текущую температуру к которой будем стремиться

      TemperatureSensor::pwm(TemperatureSensor::TemperatureSensor::regulator_AIR.getResultTimer());                  // включаем ТТР, опираясь на температуру воздуха
      TemperatureSensor::TemperatureSensor::regulator_WATER.integral = 0;
  
      if (abs(target_temperature - TemperatureSensor::current_temp) > SCATTER_TEMP){                                 // если текущая температура не достигла диапазона регулирования, недопускаем накопление интегральной ошибки
        TemperatureSensor::TemperatureSensor::regulator_AIR.integral = 0;                                            //интегральная составляющая для воздуха не должна рости
      }
    } else {
      Serial.println(F("External sensor failure"));
      TemperatureSensor::pid_off();
    }
  }
}

void TemperatureSensor::pid_off(){
  TemperatureSensor::is_heating_on = false;
  digitalWrite(SSR1_OUT_PIN, HEATER_OFF);
  digitalWrite(HEAT_LED_PIN, HIGH);
}

void TemperatureSensor::pwm(uint32_t on_time){
  uint32_t time_msec = (on_time * TemperatureSensor::period_msec) / 255;

  TemperatureSensor::pwm_set_0_time = millis() + time_msec;
  if (time_msec > 0){
    TemperatureSensor::is_heating_on = true;
    digitalWrite(SSR1_OUT_PIN, HEATER_ON);
    digitalWrite(HEAT_LED_PIN, LOW);

    if (time_msec == TemperatureSensor::period_msec){
      if (TemperatureSensor::check_ssr_last_time == 0){
        TemperatureSensor::check_ssr_last_time = millis();
        TemperatureSensor::check_ssr_last_temp = (uint8_t)TemperatureSensor::current_temp_water;
      } else if (millis() - TemperatureSensor::check_ssr_last_time >= HEATER_DEGREE_TIMEOUT){
        // если за 15мин интенсивного нагрева температура теплоносителя не изменилась, то ошибка.
        if ((uint8_t)TemperatureSensor::current_temp_water == TemperatureSensor::check_ssr_last_temp){
          // error: don't heat
          ErrorService::add_error(ERROR_NOPOWER);
          DisplayManager::set_page_name(pageError);
          return;
        }
      }
    } else  {
      TemperatureSensor::check_ssr_last_time = 0;
    }
  } else {
    TemperatureSensor::check_ssr_last_time = 0;
  }
  ErrorService::remove_error(ERROR_NOPOWER);
}

void TemperatureSensor::pid_init(){
  TemperatureSensor::TemperatureSensor::regulator_WATER.setDirection(NORMAL); // (NORMAL/REVERSE)
  TemperatureSensor::TemperatureSensor::regulator_WATER.setLimits(0, 255);    //

  TemperatureSensor::TemperatureSensor::regulator_AIR.setDirection(NORMAL); // (NORMAL/REVERSE)
  TemperatureSensor::TemperatureSensor::regulator_AIR.setLimits(0, 255);    //


  digitalWrite(SSR1_OUT_PIN, HEATER_OFF);
  pinMode(HEAT_LED_PIN, OUTPUT);
  digitalWrite(HEAT_LED_PIN, HIGH);
}

uint8_t TemperatureSensor::update_current_temp_water() {  
  float tempC = TemperatureSensor::sensors.getTempCByIndex(0);
  // Check if reading was successful
  if (tempC != DEVICE_DISCONNECTED_C) {
    TemperatureSensor::current_temp_water = tempC;
    if (TemperatureSensor::sensors.isConversionComplete()){
      TemperatureSensor::sensors.requestTemperaturesByIndex(0);
      TemperatureSensor::sens_temp_tries = 5;
      return GOT_TEMP;
    } else {
      return NO_TEMP;
    }
  } else if (TemperatureSensor::sens_temp_tries == 0) {
    Serial.println("Error: Could not read temperature data");
    ErrorService::add_error(ERROR_TEMPSENSBROKEN);
    TemperatureSensor::sens_temp_tries = 5;
    return TEMP_SENS_ERROR;
  }
  
  sens_temp_tries--;
  return NO_TEMP;
}

bool TemperatureSensor::is_radio_connected() {
  return TemperatureSensor::radio_connected == RADIO_ON || TemperatureSensor::radio_connected == RADIO_WAIT;
}

uint8_t TemperatureSensor::get_current_temperature() {
  return TemperatureSensor::current_temp;
}

float TemperatureSensor::get_current_temp_water() {
  return TemperatureSensor::current_temp_water;
}

void TemperatureSensor::set_current_temp_like_water_temp() {
  TemperatureSensor::update_current_temp_water();
  TemperatureSensor::current_temp = (uint8_t)TemperatureSensor::current_temp_water;
}

void TemperatureSensor::set_current_temp_like_air_temp() {
  TemperatureSensor::current_temp = (uint8_t)TemperatureSensor::current_temp_air;
}

float TemperatureSensor::get_radio_temp() {
  return RadioSensor::get_radio_temp();
}

uint8_t TemperatureSensor::update_radio_temp() {
  uint8_t radio_sensor_status = RadioSensor::get_current_sensor_state();
  if (radio_sensor_status == GOT_EXT_TEMP) {
    TemperatureSensor::current_temp_air = RadioSensor::get_radio_temp();
  }
  return radio_sensor_status;
}

bool TemperatureSensor::is_radio_lost() {
  return radio_connected == RADIO_LOST;
}

bool TemperatureSensor::is_radio_wait() {
  return radio_connected == RADIO_WAIT;
}

bool TemperatureSensor::is_radio_on() {
  return radio_connected == RADIO_ON;
}

void TemperatureSensor::set_radio_on() {
  TemperatureSensor::radio_connected = RADIO_ON;
}

void TemperatureSensor::set_radio_lost() {
  TemperatureSensor::radio_connected = RADIO_LOST;
}
