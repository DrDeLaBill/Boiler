#include "TemperatureSensor.h"

uint8_t TemperatureSensor::current_temp = 0;
uint8_t TemperatureSensor::radio_connected = 0;
uint8_t TemperatureSensor::sens_temp_tries = 5;
uint32_t TemperatureSensor::ds18b20_last_time = millis();
RadioSensor TemperatureSensor::radio_sensor;

TemperatureSensor::TemperatureSensor() {
  this->regulator_AIR = new GyverPID(kP_air, kI_air, kD_air, dT);
  this->regulator_WATER = new GyverPID(kP_water, kI_water, kD_water, dT);
  this->oneWire = new OneWire(ONE_WIRE_BUS);
  this->sensors = new DallasTemperature(oneWire);
  this->current_temp_water = 0;                   
  this->current_temp_air = 0;                    
  this->pid_last_time = 0;              
  this->pwm_set_0_time = 0;
  this->check_ssr_last_time = 0;       
  this->check_ssr_last_temp = 0;  
  this->temp_init();
  this->pid_init();
  Serial.println(F("Temperature sensor init"));
}

void TemperatureSensor::temp_init() {
  this->sensors->begin();
  this->sensors->setWaitForConversion(false);
  this->sensors->requestTemperatures();
}

void TemperatureSensor::check_temperature() {
  uint8_t sens_status = this->update_current_temp_water();

  if (sens_status == GOT_TEMP){
    if (this->current_temp_water >= WATER_TEMP_LIM){
      // если температура теплоносителя стала аварийно высокой
      // здесь надо отключать силовое питание!
      ErrorService::add_error(ERROR_WATEROVERHEAT);
    } else {
      // если температура понизилась, то может и не стоит возвращаться в обычный режим?
      ErrorService::clear_errors();
    }
      
    if (ErrorService::is_set_error(ERROR_TEMPSENSBROKEN)){
      // если датчик температуры был неисправен, а теперь починился
      ErrorService::clear_errors();
    }

    if (BoilerProfile::session_boiler_mode == MODE_WATER){
      this->set_current_temp_like_water_temp();
    }
  }

  if (sens_status == TEMP_SENS_ERROR){
    // датчик не выходит на связь
    ErrorService::add_error(ERROR_TEMPSENSBROKEN);
  }
  
  sens_status = this->update_radio_temp();

  if (sens_status == GOT_EXT_TEMP){
    if (this->is_radio_lost()){
      // если датчик отваливался, а теперь появился
      // проверим, надо ли нам переключить режим обратно
      //TODO: заменить проверку мода на функцию
      if (BoilerProfile::boiler_configuration.boiler_mode == MODE_AIR || BoilerProfile::boiler_configuration.boiler_mode == MODE_PROFILE){
        BoilerProfile::session_boiler_mode = BoilerProfile::boiler_configuration.boiler_mode;
      }
    }
    this->set_radio_on();
    
    if (BoilerProfile::is_mode_air() || BoilerProfile::is_mode_profile()){ //##############################################
      this->set_current_temp_like_air_temp();
    }
  } else if (sens_status == RADIO_ERROR){
    // датчика нет
    Serial.println("RADIO_ERROR");
    if (this->is_radio_on() || this->is_radio_wait()){
      // а до этого был или должен был быть
      // то переключаем режим работы на уставку по воде
      Serial.println("validate true");
      if (BoilerProfile::boiler_configuration.boiler_mode == MODE_AIR || BoilerProfile::boiler_configuration.boiler_mode == MODE_PROFILE){
        Serial.println("set boiler mode water");
        BoilerProfile::session_boiler_mode = MODE_WATER;
        BoilerProfile::session_target_temp_int = (uint8_t)this->get_current_temp_water();
        this->set_current_temp_like_water_temp();
      }
    }
    this->set_radio_lost();
  }
}

void TemperatureSensor::set_radio_sensor(uint8_t target_temperature){
  TemperatureSensor::radio_sensor.clear_timeout_radio_sens();
  if (TemperatureSensor::is_radio_connected() != RADIO_ON) {
    TemperatureSensor::radio_connected = RADIO_WAIT;
  }
  TemperatureSensor::current_temp = target_temperature;          // пока датчик не отправил данные, загружаем это значение
}

void TemperatureSensor::pid_regulating(bool is_mode_water, uint8_t target_temperature){
  if ((millis() > pwm_set_0_time) && digitalRead(SSR1_OUT_PIN)){
    digitalWrite(SSR1_OUT_PIN, HEATER_OFF);
    digitalWrite(HEAT_LED_PIN, HIGH);
  }

  // отправка показаний с датчиков на пид регулятор с определенной периодичностью
  if (millis() - this->pid_last_time > 1000){
    this->pid_last_time = millis();

    // разные пиды для режимов работы по воздуху и теплоносителю
    if (is_mode_water) {                                                                  // работать по воде
      this->regulator_WATER->setpoint = target_temperature;                           // Сообщаем регулятору температуру к которой следует стремиться
      this->regulator_WATER->input = TemperatureSensor::current_temp;                                   // Сообщаем регулятору текущую температуру к которой будем стремиться
      
      this->pwm(this->regulator_WATER->getResultTimer());                                  // включаем ТТР, опираясь на температуру воды

      this->regulator_AIR->integral = 0;                                             //интегральная составляющая для воздуха не должна рости

      if (abs(target_temperature - TemperatureSensor::current_temp) > SCATTER_TEMP){               // если текущая температура не достигла диапазона регулирования, недопускаем накопление интегральной ошибки
        this->regulator_WATER->integral = 0;
      }
    } else {
      // если сейчас работаем по воздуху или термопрофилю

      this->regulator_AIR->setpoint = target_temperature;                             // Сообщаем регулятору температуру к которой следует стремиться
      this->regulator_AIR->input = TemperatureSensor::current_temp;                                     // Сообщаем регулятору текущую температуру к которой будем стремиться

      this->pwm(this->regulator_AIR->getResultTimer());                                    // включаем ТТР, опираясь на температуру воздуха
      this->regulator_WATER->integral = 0;

      if (abs(target_temperature - TemperatureSensor::current_temp) > SCATTER_TEMP){               // если текущая температура не достигла диапазона регулирования, недопускаем накопление интегральной ошибки
        this->regulator_AIR->integral = 0;                                           //интегральная составляющая для воздуха не должна рости
      }
    }
  }
}

void TemperatureSensor::pid_off(void){
  digitalWrite(SSR1_OUT_PIN, HEATER_OFF);
  digitalWrite(HEAT_LED_PIN, HIGH);
}

void TemperatureSensor::pwm(uint32_t on_time){
  uint32_t time_msec = (on_time * this->period_msec) / 255;

  this->pwm_set_0_time = millis() + time_msec;
  if (time_msec > 0){
    digitalWrite(SSR1_OUT_PIN, HEATER_ON);
    digitalWrite(HEAT_LED_PIN, LOW);

    if (time_msec == this->period_msec){
      if (this->check_ssr_last_time == 0){
        this->check_ssr_last_time = millis();
        this->check_ssr_last_temp = (uint8_t)this->current_temp_water;
      } else if (millis() - this->check_ssr_last_time >= HEATER_1DEGREE_TIMEOUT){
        // если за 15мин интенсивного нагрева температура теплоносителя не изменилась, то ошибка.
        if ((uint8_t)this->current_temp_water == this->check_ssr_last_temp){
          // error: don't heat
          ErrorService::add_error(ERROR_NOPOWER);
        }
      }
    } else  {
      this->check_ssr_last_time = 0;
    }
  } else {
    this->check_ssr_last_time = 0;
  }
}

void TemperatureSensor::pid_init(){
  this->regulator_WATER->setDirection(NORMAL); // (NORMAL/REVERSE)
  this->regulator_WATER->setLimits(0, 255);    //

  this->regulator_AIR->setDirection(NORMAL); // (NORMAL/REVERSE)
  this->regulator_AIR->setLimits(0, 255);    //

  digitalWrite(SSR1_OUT_PIN, HEATER_OFF);
  pinMode(HEAT_LED_PIN, OUTPUT);
  digitalWrite(HEAT_LED_PIN, HIGH);
}

uint8_t TemperatureSensor::update_current_temp_water() {
  // считывание данных с датчика температуры с некоторой периодичностью
  if (millis() - TemperatureSensor::ds18b20_last_time >= DS18B20_MEAS_PERIOD){
    TemperatureSensor::ds18b20_last_time = millis();
    
    float tempC = sensors->getTempCByIndex(0);
    // Check if reading was successful
    if (tempC != DEVICE_DISCONNECTED_C) {
      this->current_temp_water = tempC;
      if (sensors->isConversionComplete()){
        sensors->requestTemperaturesByIndex(0);
        TemperatureSensor::sens_temp_tries = 5;
        return GOT_TEMP;
      } else {
        return NO_TEMP;
      }
    } else if (TemperatureSensor::sens_temp_tries == 0) {
      Serial.println("Error: Could not read temperature data");
      TemperatureSensor::sens_temp_tries = 5;
      return TEMP_SENS_ERROR;
    } else {
      sens_temp_tries--;
      return NO_TEMP;
    }
  }
  return NO_TEMP;
}

bool TemperatureSensor::is_radio_connected() {
  return TemperatureSensor::radio_connected;
}

uint8_t TemperatureSensor::get_current_temperature() {
  return TemperatureSensor::current_temp;
}

float TemperatureSensor::get_current_temp_water() {
  return this->current_temp_water;
}

void TemperatureSensor::set_current_temp_like_water_temp() {
  TemperatureSensor::current_temp = (uint8_t)this->current_temp_water;
}

void TemperatureSensor::set_current_temp_like_air_temp() {
  TemperatureSensor::current_temp = (uint8_t)this->current_temp_air;
}

//TODO: убрать указатель
float TemperatureSensor::get_radio_temp() {
  return TemperatureSensor::radio_sensor.get_radio_temp();
}

uint8_t TemperatureSensor::update_radio_temp() {
  uint8_t radio_sensor_status = TemperatureSensor::radio_sensor.update_radio_temp();
  if (radio_sensor_status == GOT_EXT_TEMP) {
    this->current_temp_air = TemperatureSensor::radio_sensor.get_radio_temp();
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
