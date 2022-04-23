/*
 * 
 * Работа с температурами и регулированием.
 * 
 */

#ifndef _TEMPERATURE_SENSOR_H_
#define _TEMPERATURE_SENSOR_H_

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "GyverPID.h"

#include "BoilerConstants.h"
#include "RadioSensor.h"
#include "BoilerProfile.h"

class TemperatureSensor
{
  public:
    //пиды по воздуху
    static float kP_air;                             //Пропорциональная составляющая
    static float kI_air;                           //Интегральная составляющая
    static float kD_air;                           //Дифференциальная составляющая
    //пиды по воде
    static float kP_water;                           //Пропорциональная составляющая
    static float kI_water;                         //Интегральная составляющая
    static float kD_water;                         //Дифференциальная составляющая
    //Время расчета для обоих регуляторов
    static float dT;
    // время, за которое переключается включение/выключение ТТР
    static const uint32_t period_msec;
    /*
     * 
     * в процессе работы можно менять коэффициенты
     * regulator.Kp = 5.2;
     * regulator.Ki += 0.5;
     * regulator.Kd = 0;
     * 
     */
    static GyverPID regulator_AIR;
    static GyverPID regulator_WATER;
    static OneWire oneWire;
    static DallasTemperature sensors;
    // текущая температура теплоносителя                         
    static float current_temp_water;     
    // текущая температура с внешнего датчика                
    static float current_temp_air;    
    // хранение времени для периода передачи текущей температуры в пид регулятор                   
    static uint32_t pid_last_time;       
    // Время, когда надо выключить TTP                  
    static uint32_t pwm_set_0_time;           
    // для оценки нагрева теплоносителя
    static uint32_t check_ssr_last_time;                   
    // для оценки нагрева теплоносителя
    static uint8_t check_ssr_last_temp;
    // текущая температура для отображения и ПИД
    static uint8_t current_temp;
    // статус подключения внешнего датчика           
    static uint8_t radio_connected;
    // количество попыток чтения
    static uint8_t sens_temp_tries;
    // хранение времени для периода датчика
    static uint32_t ds18b20_last_time;
    
    TemperatureSensor();
    static void set_radio_sensor(uint8_t target_temperature);
    static bool is_radio_connected();
    static uint8_t get_current_temperature();
    static void check_temperature();
    static void temp_init();
    static uint8_t update_current_temp_water();
    static void pid_off();
    static void pid_init();
    static void pwm(uint32_t time_on);
    static void pid_regulating(bool is_mode_water, uint8_t target_temperature);
    static float get_current_temp_water();
    static void set_current_temp_like_water_temp();
    static void set_current_temp_like_air_temp();
    static float get_radio_temp();
    static uint8_t update_radio_temp();
    static bool is_radio_lost();
    static bool is_radio_wait();
    static bool is_radio_on();
    static void set_radio_on();
    static void set_radio_lost();
};

#endif
