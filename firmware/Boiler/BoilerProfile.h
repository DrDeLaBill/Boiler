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
    void _serial_print_boiler_configuration();
  public:
    static ClockRTC clock_rtc;
    // требуемая температура теплоносителя (в данной сессии)
    static uint8_t session_target_temp_int;
    // режим работы котла (в данной сессии)
    static uint8_t session_boiler_mode;
    static BoilerConfiguration boiler_configuration;

    BoilerProfile();
    static void save_configuration();
    static uint8_t get_target_temp();
    static String get_boiler_id();
    static uint8_t period_of_day();
    static void set_boiler_mode(uint8_t target_mode);
    static void set_target_temp(uint8_t temp);
    static void set_boiler_id(String boiler_id);
    static bool is_set_session_boiler_mode(ModeType search_mode);
    static bool is_set_config_boiler_mode(ModeType search_mode);
    static void clear_eeprom();
    static void start_eeprom();
    static void set_default_settings();
    static uint8_t get_session_boiler_mode();
    static String get_ssid();
    static String get_pass();
    static void set_wifi_settings(String ssid, String pass);
    static uint8_t get_profile_for_week_day();
    static void set_config_day_profile(uint8_t day, uint8_t value);
    static void set_day_preset(uint8_t day_number, uint8_t day_period, uint8_t value);
    static void set_session_boiler_mode(uint8_t target_mode);
    static void set_settings_standby(bool state);
    static void check_temperature();
    static char *get_current_day(const char* fmt);
    static char *get_current_time(const char* fmt);
    static BoilerConfiguration get_boiler_configuration();
    static void print_configuration_symbol(byte symbol);
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
