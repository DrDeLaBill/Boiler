/* 
 *  Чтение, запись термопрофиля в eeprom.
 *  Получение текущей уставки температуры.
 *  Получение текущих даты и времени.
 */
#ifndef _BOILER_PROFILE_H_
#define _BOILER_PROFILE_H_

#include <Arduino.h>
#include "stdint.h"
#include "EEPROM.h"

#include "BoilerConstants.h"
#include "ErrorService.h"
#include "TemperatureSensor.h"
#include "ClockRTC.h"

class BoilerProfile
{
  private:
    ClockRTC *clock_rtc;
    TemperatureSensor *temperature_sensor;
    
    void _serial_print_boiler_configuration();
  public:
    // требуемая температура теплоносителя (в данной сессии)
    static uint8_t session_target_temp_int;
    // режим работы котла (в данной сессии)
    static uint8_t session_boiler_mode;
    static BoilerConfiguration boiler_configuration;
    static void save_configuration();
    static uint8_t get_target_temp();
    static String get_boiler_id();
    static uint8_t period_of_day();
    static void set_boiler_mode(uint8_t target_mode);
    static void set_target_temp(uint8_t temp);
    static void set_boiler_id(String boiler_id);
    static bool is_mode_air();
    static bool is_mode_water();
    static bool is_mode_profile();
    static void clear_eeprom();
    static void start_eeprom();
    static void set_default_settings();
    static uint8_t get_session_boiler_mode();
    
    BoilerProfile();
    void set_session_boiler_mode(uint8_t target_mode);
    void set_settings_standby(bool state);
    void set_config_day_profile(uint8_t day, uint8_t value);
    uint8_t get_profile_for_week_day();
    void temperature_pid_regulating();
    void temperature_pid_off();
    void check_temperature();
    String get_ssid();
    String get_pass();
    void set_wifi_settings(String ssid, String pass);
    bool is_radio_connected();
    uint8_t get_current_temperature();
    char *get_current_day(const char* fmt);
    char *get_current_time(const char* fmt);
    uint8_t get_temperature_error();
    void set_day_preset(uint8_t day_number, uint8_t day_period, uint8_t value);
    BoilerConfiguration get_boiler_configuration();
};

#endif

/*
 * что нам нужно хранить в конфиге?
 * текущий режим работы: вода, воздух, термопрофиль(воздух)
 * сам термопрофиль
 * заданную уставку по теплоносителю
 * заданную уставку по воздуху
 * флаг, работал ли котел до отключения питания
 */

/*
 * boiler_mode: заданныый режим работы
 * target_temp_int: установленная пользователем температура теплоносителя
 * target_temp_ext: установленная пользователем температура воздуха
 * standby_flag: включен котел или в режиме ожидания после подачи питания
 * ssid, password - для подключения к роутеру
 * 
 * profile[7]: термопрофиль 7 дней, хранит номера пресетов
 * presets[5][6] = пресеты, храним выставленные температуры в периодах
 * 
*/
