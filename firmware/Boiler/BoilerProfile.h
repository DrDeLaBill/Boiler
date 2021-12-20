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
#include "ClockRTC.h"
#include "temp.h"

#define STANDBY                     false
#define WORK                        true

#define TARGET_TEMP_EXT_DEFAULT     20      // установленная температура воздуха по умолчанию
#define TARGET_TEMP_INT_DEFAULT     10      // установленная температура теплоносителя по умолчанию

#define MAX_SIZE_SSID               20
#define MAX_SIZE_PASS               20      // ограничение по длине
#define NAME_MAX_SIZE               64    
#define ID_MAX_SIZE                 11

#define NUM_DAYS                    7       // термопрофиль на 7 дней
#define NUM_PERIODS                 6       // 6 периодов в сутках
#define NUM_PRESETS                 4       // общее количество возможных пресетов

#define PRESET_WEEKDAY              0       // пресеты, по ним адресуются значения в массиве profile[]
#define PRESET_WEEKEND              1
#define PRESET_CUSTOM               2
#define PRESET_NOTFREEZE            3

struct BoilerConfiguration {
  uint8_t boiler_mode;
  uint8_t target_temp_int;
  uint8_t target_temp_ext;
  bool standby_flag;
  char ssid[MAX_SIZE_SSID];
  char password[MAX_SIZE_PASS];
  uint8_t profile[NUM_DAYS];
  uint8_t presets[NUM_PRESETS][NUM_PERIODS];
  char boiler_id[ID_MAX_SIZE];
  char boiler_name[NAME_MAX_SIZE];
};

enum {
  MODE_WATER,
  MODE_AIR,
  MODE_PROFILE
};

class BoilerProfile
{
  private:
    uint8_t user_target_temp_int = 0;   // требуемая температура теплоносителя (в данной сессии)
    uint8_t user_boiler_mode = 0;       // режим работы котла (в данной сессии)
    BoilerConfiguration boiler_configuration;
    ClockRTC clock_rtc;
    
    void _start_eeprom();
    void _serial_print_boiler_configuration();
  public:
    BoilerProfile();
    uint8_t get_target_temp();
    uint8_t period_of_day();
    void clear_eeprom();
    void save_configuration();
    void set_target_temp(uint8_t temp);
    void set_boiler_mode(uint8_t target_mode);
    void set_default_settings();
    void set_settings_standby(bool state);
    void set_boiler_id(String boiler_id);
    String get_boiler_id();
    String get_ssid();
    String get_pass();
    void set_wifi_settings(String ssid, String pass);
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
