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
  private:
    //пиды по воздуху
    float kP_air = 7;                             //Пропорциональная составляющая
    float kI_air = 0.5;                           //Интегральная составляющая
    float kD_air = 0.3;                           //Дифференциальная составляющая
    
    //пиды по воде
    float kP_water = 7;                           //Пропорциональная составляющая
    float kI_water = 0.5;                         //Интегральная составляющая
    float kD_water = 0.3;                         //Дифференциальная составляющая
    
    float dT = 1000;                              //Время расчета для обоих регуляторов
    
    const uint32_t period_msec = 1000;            // время, за которое переключается включение/выключение ТТР
    
    /*
     * 
     * в процессе работы можно менять коэффициенты
     * regulator.Kp = 5.2;
     * regulator.Ki += 0.5;
     * regulator.Kd = 0;
     * 
     */
    GyverPID *regulator_AIR;
    GyverPID *regulator_WATER;
    
    OneWire *oneWire;
    DallasTemperature *sensors;
     
    // текущая температура теплоносителя                         
    float current_temp_water;     
    // текущая температура с внешнего датчика                
    float current_temp_air;    
    // хранение времени для периода передачи текущей температуры в пид регулятор                   
    uint32_t pid_last_time;       
    // Время, когда надо выключить TTP                  
    uint32_t pwm_set_0_time;           
    // для оценки нагрева теплоносителя
    uint32_t check_ssr_last_time;                   
    // для оценки нагрева теплоносителя
    uint8_t check_ssr_last_temp;

    uint8_t error;
  public:
    static RadioSensor radio_sensor;
    // текущая температура для отображения и ПИД
    static uint8_t current_temp;
    // статус подключения внешнего датчика           
    static uint8_t radio_connected;
    // количество попыток чтения
    static uint8_t sens_temp_tries;
    // хранение времени для периода датчика
    static uint32_t ds18b20_last_time;
    static void set_radio_sensor(uint8_t target_temperature);
    static bool is_radio_connected();
    
    TemperatureSensor();
    void check_temperature();
    void temp_init();
    uint8_t update_current_temp_water();
    void pid_off();
    void pid_init();
    void pwm(uint32_t time_on);
    void pid_regulating(bool is_mode_water, uint8_t target_temperature);
    uint8_t get_current_temperature();
    float get_current_temp_water();
    void set_current_temp_like_water_temp();
    void set_current_temp_like_air_temp();
    float get_radio_temp();
    uint8_t update_radio_temp();
    bool is_radio_lost();
    bool is_radio_wait();
    bool is_radio_on();
    void set_radio_on();
    void set_radio_lost();
};

#endif
