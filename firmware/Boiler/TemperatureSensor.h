/*
 * 
 * Работа с температурами и регулированием.
 * 
 */

#ifndef _TEMP_H_
#define _TEMP_H_

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "GyverPID.h"
//TODO: include
#include "tprofile.h"
#include "ext_temp.h"
#include "errors.h"

#define SSR1_OUT_PIN           4           // пин включения ТТ реле
#define SSR2_OUT_PIN           16
#define SSR3_OUT_PIN           17

#define HEAT_LED_PIN           0

#define ONE_WIRE_BUS           27        // пин подключения датчика температуры ds18b20

#define WATER_TEMP_MIN        10        // минимальная температура теплоносителя
#define WATER_TEMP_MAX        60        // максимальная температура теплоносителя
#define WATER_TEMP_LIM        85.0f        // аварийная температура теплоносителя - 85*

#define AIR_TEMP_MIN          10        // минимальная температура в комнате
#define AIR_TEMP_MAX          40        // максимальная температура в комнате

#define GOT_TEMP              1
#define NO_TEMP               2
#define TEMP_SENS_ERROR       3

#define RADIO_ON              1
#define RADIO_LOST            2
#define RADIO_WAIT            3

#define DS18B20_MEAS_PERIOD    1000     // период измерения с датчика DS18B20 в мс
#define HEATER_1DEGREE_TIMEOUT 900000

#define SCATTER_TEMP           10       // Разброс температур между текущей температурой и температурой, к которой стремимся

#define HEATER_ON     HIGH
#define HEATER_OFF    LOW

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
      // в процессе работы можно менять коэффициенты
      regulator.Kp = 5.2;
      regulator.Ki += 0.5;
      regulator.Kd = 0;
    
      !!! */
    GyverPID regulator_AIR(kP_air, kI_air, kD_air, dT);
    GyverPID regulator_WATER(kP_water, kI_water, kD_water, dT);
    
    OneWire oneWire(ONE_WIRE_BUS);
    DallasTemperature sensors(&oneWire);
    
    // текущая температура для отображения и ПИД
    uint8_t current_temp = 0;  
    // текущая температура теплоносителя                         
    float current_temp_water = 0;     
    // текущая температура с внешнего датчика                
    float current_temp_air = 0;    
    // хранение времени для периода передачи текущей температуры в пид регулятор                   
    uint32_t pid_last_time = 0;       
    // Время, когда надо выключить TTP                  
    uint32_t pwm_set_0_time = 0;             
    // статус подключения внешнего датчика           
    uint8_t radio_connected = 0;                        
    // для оценки нагрева теплоносителя
    uint32_t check_ssr_last_time = 0;                   
    // для оценки нагрева теплоносителя
    uint8_t check_ssr_last_temp = 0;                    
  public:
    TemperatureSensor();
    void temp_init();
    void check_temp();
    uint8_t get_int_temp(float* pTemp);
    void pid_off();
    void pid_init();
    void pwm(uint32_t time_on);
    void pid_regulating();
    void set_ext_sensor();
};

#endif
