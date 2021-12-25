#include "TemperatureSensor.h"

TemperatureSensor::TemperatureSensor() {
//  this->kP_air = 7;
//  this->kI_air = 0.5;
//  this->kD_air = 0.3;
//  this->kP_water = 7;
//  this->kI_water = 0.5;
//  this->kD_water = 0.3;
//  this->dT = 1000;
  this->current_temp = 0;                      
  this->current_temp_water = 0;                   
  this->current_temp_air = 0;                    
  this->pid_last_time = 0;              
  this->pwm_set_0_time = 0;                   
  this->radio_connected = 0;             
  this->check_ssr_last_time = 0;       
  this->check_ssr_last_temp = 0;  
  this->temp_init();
}

//TODO: extern
extern BoilerConfig BoilerCfg;
extern DisplayPages page;
extern bool redraw_display;                         // флаг перерисовки дисплея
extern uint8_t user_target_temp_int;
extern uint8_t user_boiler_mode;
extern uint8_t user_error;

void temp_init() {
  ext_temp_init();
  this->sensors.begin();
  this->sensors.setWaitForConversion(false);
  this->sensors.requestTemperatures();
}

void set_ext_sensor(void){
  clear_timeout_ext_sens();
  if (radio_connected != RADIO_ON) radio_connected = RADIO_WAIT;
  current_temp = get_target_temp();          // пока датчик не отправил данные, загружаем это значение
}

void pid_regulating(){
  if ((millis() > pwm_set_0_time) && digitalRead(SSR1_OUT_PIN)){
    digitalWrite(SSR1_OUT_PIN, HEATER_OFF);
    digitalWrite(HEAT_LED_PIN, HIGH);
    redraw_display = true;
  }

  // отправка показаний с датчиков на пид регулятор с определенной периодичностью
  if (millis() - pid_last_time > 1000){
    pid_last_time = millis();

    // разные пиды для режимов работы по воздуху и теплоносителю
    if (user_boiler_mode == MODE_AIR || user_boiler_mode == MODE_PROFILE){
      // если сейчас работаем по воздуху или термопрофилю

      regulator_AIR.setpoint = get_target_temp();                             // Сообщаем регулятору температуру к которой следует стремиться
      regulator_AIR.input = current_temp;                                     // Сообщаем регулятору текущую температуру к которой будем стремиться

      pwm(regulator_AIR.getResultTimer());                                    // включаем ТТР, опираясь на температуру воздуха
      regulator_WATER.integral = 0;

      if (abs(get_target_temp() - current_temp) > SCATTER_TEMP){               // если текущая температура не достигла диапазона регулирования, недопускаем накопление интегральной ошибки
        regulator_AIR.integral = 0;                                           //интегральная составляющая для воздуха не должна рости
      }
    } else if (user_boiler_mode == MODE_WATER){                                                                  // работать по воде
      regulator_WATER.setpoint = get_target_temp();                           // Сообщаем регулятору температуру к которой следует стремиться
      regulator_WATER.input = current_temp;                                   // Сообщаем регулятору текущую температуру к которой будем стремиться
      
      pwm(regulator_WATER.getResultTimer());                                  // включаем ТТР, опираясь на температуру воды

      regulator_AIR.integral = 0;                                             //интегральная составляющая для воздуха не должна рости

      if (abs(get_target_temp() - current_temp) > SCATTER_TEMP){               // если текущая температура не достигла диапазона регулирования, недопускаем накопление интегральной ошибки
        regulator_WATER.integral = 0;
      }
    }
  }
}

void pid_off(void){
  digitalWrite(SSR1_OUT_PIN, HEATER_OFF);
  digitalWrite(HEAT_LED_PIN, HIGH);
  redraw_display = true;
}

void pwm(uint32_t on_time){
  uint32_t time_msec = (on_time * period_msec) / 255;

  pwm_set_0_time = millis() + time_msec;
  if (time_msec > 0){
    redraw_display = true;
    digitalWrite(SSR1_OUT_PIN, HEATER_ON);
    digitalWrite(HEAT_LED_PIN, LOW);

    if (time_msec == period_msec){
      if (check_ssr_last_time == 0){
        check_ssr_last_time = millis();
        check_ssr_last_temp = (uint8_t)current_temp_water;
      } else if (millis() - check_ssr_last_time >= HEATER_1DEGREE_TIMEOUT){
        // если за 15мин интенсивного нагрева температура теплоносителя не изменилась, то ошибка.
        if ((uint8_t)current_temp_water == check_ssr_last_temp){
          // error: don't heat
          page = pageError;
          user_error = NOPOWER;
          redraw_display = true;
        }
      }
    } else check_ssr_last_time = 0;
  } else check_ssr_last_time = 0;
  
  
}

void pid_init(){
  regulator_WATER.setDirection(NORMAL); // (NORMAL/REVERSE)
  regulator_WATER.setLimits(0, 255);    //

  regulator_AIR.setDirection(NORMAL); // (NORMAL/REVERSE)
  regulator_AIR.setLimits(0, 255);    //

  digitalWrite(SSR1_OUT_PIN, HEATER_OFF);
  pinMode(HEAT_LED_PIN, OUTPUT);
  digitalWrite(HEAT_LED_PIN, HIGH);
}

void check_temp(){

  uint8_t sens_status = get_int_temp(&current_temp_water);

  if (sens_status == GOT_TEMP){
    if (current_temp_water >= WATER_TEMP_LIM && user_error != WATEROVERHEAT){
        // если температура теплоносителя стала аварийно высокой
        // здесь надо отключать силовое питание!
        page = pageError;
        user_error = WATEROVERHEAT;
        redraw_display = true;
        digitalWrite(CRASH_OUT_PIN, HIGH);
    } else if (user_error == WATEROVERHEAT && current_temp_water < WATER_TEMP_LIM){
      // если температура понизилась, то может и не стоит возвращаться в обычный режим?
      
//      page = pageTemp;
//      user_error = NOERROR;
//      redraw_display = true;
    }
      
    if (user_error == TEMPSENSBROKEN){
      // если датчик температуры был неисправен, а теперь починился
      page = pageTemp;
      user_error = NOERROR;
      redraw_display = true;
    }

    if (user_boiler_mode == MODE_WATER){
      current_temp = (uint8_t)current_temp_water;
    }
    
  }

  if (sens_status == TEMP_SENS_ERROR && user_error != TEMPSENSBROKEN){
    // датчик не выходит на связь
    page = pageError;
    user_error = TEMPSENSBROKEN;
    redraw_display = true;
  }
  
  sens_status = get_ext_temp(&current_temp_air);

  if (sens_status == GOT_EXT_TEMP){
    if (radio_connected == RADIO_LOST){
      // если датчик отваливался, а теперь появился
      // проверим, надо ли нам переключить режим обратно
      if (BoilerCfg.boiler_mode == MODE_AIR || BoilerCfg.boiler_mode == MODE_PROFILE){
        user_boiler_mode = BoilerCfg.boiler_mode;
      }
    }
    radio_connected = RADIO_ON;
    
    if (user_boiler_mode == MODE_AIR || user_boiler_mode == MODE_PROFILE){
      current_temp = (uint8_t)current_temp_air;
    }
  } else if (sens_status == RADIO_ERROR){
    // датчика нет
    if (radio_connected == RADIO_ON || radio_connected == RADIO_WAIT){
      // а до этого был или должен был быть
      // то переключаем режим работы на уставку по воде
      if (BoilerCfg.boiler_mode == MODE_AIR || BoilerCfg.boiler_mode == MODE_PROFILE){
        user_boiler_mode = MODE_WATER;
        user_target_temp_int = (uint8_t)current_temp_water;
        current_temp = (uint8_t)current_temp_water;
        redraw_display = true;
      }
    }
    radio_connected = RADIO_LOST;
  }
}

uint8_t get_int_temp(float* pTemp) {
  // считывание данных с датчика температуры с некоторой периодичностью

  static uint8_t sens_temp_tries = 5;       // количество попыток чтения
  static uint32_t ds18b20_last_time = millis();    // хранение времени для периода датчика

  if (millis() - ds18b20_last_time >= DS18B20_MEAS_PERIOD){
    ds18b20_last_time = millis();
    
    float tempC = sensors.getTempCByIndex(0);  
    
    // Check if reading was successful
    if (tempC != DEVICE_DISCONNECTED_C) {
      *pTemp = tempC;
      if (sensors.isConversionComplete()){
        sensors.requestTemperaturesByIndex(0);
        sens_temp_tries = 5;
        return GOT_TEMP;
      } else return NO_TEMP;
      
    } else if (sens_temp_tries == 0) {
      Serial.println("Error: Could not read temperature data");
      sens_temp_tries = 5;
      return TEMP_SENS_ERROR;
      
    } else {
      sens_temp_tries--;
      return NO_TEMP;
    }
  }
  return NO_TEMP;
}

bool is_radio_connected() {
  return this->radio_connected();
}

uint8_t get_current_temperature() {
  return this->current_temp;
}
