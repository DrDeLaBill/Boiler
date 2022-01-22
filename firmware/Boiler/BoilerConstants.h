/*
 * Константы для всего проекта
 */
#ifndef _BOILER_CONSTANTS_H_
#define _BOILER_CONSTANTS_H_

#include <Arduino.h>

// Ошибки
#define ERRORS_COUNT                  7           // Изменить, если количество поменяется

#define ERROR_NOERROR                 0           // нет ошибок
#define ERROR_OVERHEAT                1           // перегрев (аварийный датчик), которого нет
#define ERROR_PUMPBROKEN              2           // насос неисправен
#define ERROR_SSRBROKEN               3           // твердотельные реле не выключаются
#define ERROR_TEMPSENSBROKEN          4           // датчик температуры теплоносителя не работает
#define ERROR_WATEROVERHEAT           5           // перегрев теплоносителя (обычный датчик)
#define ERROR_NOPOWER                 6           // нет нагрева

// Состояния кнопки-энкодера
#define BUTTON_NO_PRESSED             0
#define BUTTON_PRESSED                1
#define BUTTON_HOLDED                 2
#define BUTTON_RELEASED               3
#define BUTTON_NON_ROTARY             0
#define BUTTON_ROTARY_RIGHT           1
#define BUTTON_ROTARY_LEFT            2

// Настройки для температурного сенсора
#define AIR_TEMP_MIN                  10          // минимальная температура в комнате
#define AIR_TEMP_MAX                  40          // максимальная температура в комнате

// Пины
#define SSR1_OUT_PIN                  4           // пин включения ТТ реле
#define SSR2_OUT_PIN                  16
#define SSR3_OUT_PIN                  17
#define SSR_IN_PIN                    3
#define PUMP_OUT_PIN                  32          // пин на насос
#define FLOW_IN_PIN                   34          // вход с датчика протока

//Настройки сети (network manager)
#define ARDUINOJSON_USE_LONG_LONG     1
#define WIFI_CONNECT_TIMEOUT          5000        // таймаут подключения к WiFi сети
#define WEB_REQUESTS_PERIOD           10000       // период отправки статуса на веб-сервер

enum ConnectStatus {
  DISCONNECTED = 0,
  CONNECTED,
  SETS_NOT_SENDED
};

// Настройки профиля бойлера (BoilerPRofile)

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

// Настройки температурного сенсора (TemperatureSensor)
#define HEAT_LED_PIN            0

#define ONE_WIRE_BUS            27          // пин подключения датчика температуры ds18b20

#define WATER_TEMP_MIN          10          // минимальная температура теплоносителя
#define WATER_TEMP_MAX          60          // максимальная температура теплоносителя
#define WATER_TEMP_LIM          85.0f       // аварийная температура теплоносителя - 85*

#define GOT_TEMP                1
#define NO_TEMP                 2
#define TEMP_SENS_ERROR         3

#define RADIO_ON                1
#define RADIO_LOST              2
#define RADIO_WAIT              3

#define DS18B20_MEAS_PERIOD     1000        // период измерения с датчика DS18B20 в мс
#define HEATER_1DEGREE_TIMEOUT  900000

#define SCATTER_TEMP            10          // Разброс температур между текущей температурой и температурой, к которой стремимся

#define HEATER_ON               HIGH
#define HEATER_OFF              LOW

// Режимы работы бойлера
enum {
  MODE_WATER,
  MODE_AIR,
  MODE_PROFILE
};

// Базовые настройки бойлера
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

/*
 * параметры переменных для REST API
 * (настройки для NetworkManager)
 */
const char* S_SSID = "ssid";
const char* S_PASSWORD = "password";
const char* S_DT = "dt";
const char* S_TZ = "tz";
const char* S_NAME = "name";
const char* S_WEEKDAYS = "weekdays";
const char* S_WEEKEND = "weekend";
const char* S_CUSTOM = "custom";
const char* S_NOTFREEZE = "notfreeze";
const char* S_H0 = "h0";
const char* S_H1 = "h1";
const char* S_H2 = "h2";
const char* S_H3 = "h3";
const char* S_H4 = "h4";
const char* S_H5 = "h5";
const char* S_SETPOINT  = "setpoint";
const char* S_SETPOINTWATER = "setpointwater";
const char* S_PROFILE = "profile";

const char* presets[NUM_PRESETS] = {S_WEEKDAYS, S_WEEKEND, S_CUSTOM, S_NOTFREEZE};

const char* http_login = "boilerClient";    // логин и пароль для входа на сайт
const char* http_pass = "df78jhl<z";

//Рисунки на дисплее
const unsigned char pict_air[] = {
  0x0, 0x0, 0x0, 0x1e, 0x0, 0x33, 0x0, 0x61, 0x0, 0x40, 0xfe, 0x7f, 0x0, 0x0, 0xfe, 0x1f,
  0x0, 0x30, 0x3e, 0x60, 0x60, 0x40, 0x40, 0x60, 0x48, 0x32, 0x78, 0x1e, 0x0, 0x0, 0x0, 0x0
};

const unsigned char pict_water[] = {
  0x0, 0x0, 0x0, 0x0, 0xc, 0x43, 0x92, 0x24, 0x61, 0x18, 0xc, 0x43, 0x92, 0x24, 0x61, 0x18,
  0xc, 0x43, 0x92, 0x24, 0x61, 0x18, 0xc, 0x43, 0x92, 0x24, 0x61, 0x18, 0x0, 0x0, 0x0, 0x0
};

const unsigned char pict_therm[] = {
  0xc, 0x0, 0xec, 0x1, 0xc, 0x0, 0xec, 0x0, 0xc, 0x0, 0x6c, 0x0, 0xc, 0x0, 0x2c, 0x0,
  0xc, 0x0, 0xc, 0x0, 0xc, 0x0, 0x1e, 0x0, 0x21, 0x0, 0x21, 0x0, 0x33, 0x0, 0x1e, 0x0
};

const unsigned char wifi[] = {
  0xe0, 0x0, 0x18, 0x3, 0x4, 0x4, 0xf2, 0x9, 0x8, 0x2, 0xe4,
  0x4, 0x10, 0x1, 0x0, 0x0, 0x40, 0x0, 0x0, 0x0, 0x0, 0x0
};

const unsigned char heat[] = {
  0xff, 0x1, 0xaa, 0x0, 0x0, 0x0, 0xaa, 0x0, 0xaa,
  0x0, 0xaa, 0x0, 0xaa, 0x0, 0xff, 0x1, 0xaa, 0x0
};

const unsigned char inet[] = {
  0x28, 0x68, 0xe8, 0x28, 0x28, 0x28, 0x2e, 0x2c, 0x28
};

// Страницы дисплея
enum DisplayPages {
  pageTemp,
  pageTempSet,
  pageSaveSettings,
  pageSettings,
  pageSetMode,
  pageError,
  pageResetSettings
};

#endif
