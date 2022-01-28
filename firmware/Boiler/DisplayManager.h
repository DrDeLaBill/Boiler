#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <Arduino.h>
//модифицированная версия для больших шрифтов
#include <U8g2lib.h> 

#include "BoilerConstants.h"

#define LED_PIN                     23      // пин подсветки дисплея

#define CANCEL_TIMEOUT              5000    // время возврата на основной экран, если не было активности, мс
#define SAVE_TIMEOUT                1300    // время показа экрана "сохранено", мс

#define TIMEOUT_LIGHTNING           20000   // время до автоматического выключения подсветки

struct DisplayDataConfig {
  bool is_wifi_connect;
  bool is_heating_on;
  bool is_connected_to_server;
  bool is_external_sensor;
  bool is_internal_sensor;
  bool is_radio_connected;
  bool is_overheat;
  bool is_pumpbroken;
  bool is_ssrbroken;
  bool is_tempsensbroken;
  bool is_nopower;
  uint8_t current_temperature;
  uint8_t target_temperature;
  char* current_day;
  char* current_time;
};

class DisplayManager
{
  private:
    DisplayDataConfig display_data_config;
    
    //U8G2_PCD8544_84X48_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 14, /* data=*/ 27, /* cs=*/ 26, /* dc=*/ 25, /* reset=*/ 33);  // Nokia 5110 Display
    U8G2_PCD8544_84X48_F_4W_HW_SPI *u8g2;  // Nokia 5110 Display
  public:
    static DisplayPages page_name;
    static uint8_t brightness;
    // если в течении 5с не было изменений, то отмена TODO: почему эта переменная ещё и extern
    static uint32_t t_newPage;
    // настраиваемая температура
    static uint8_t temporary_target_temp; 
    // отображать "Сохранено" на 1с
    static uint32_t t_page_save_settings;
    // позиция рамки в меню
    static uint8_t menu_item;   
    
    static void set_page_name(DisplayPages page_name);
    static const char* presets[NUM_PRESETS];
    static void rotary_right(uint8_t session_boiler_mode);
    static void rotary_left(uint8_t session_boiler_mode);
    static void set_t_newPage(int value);
    static DisplayPages get_page_name();
    static void set_temporary_target_temp(uint8_t temporary_target_temp);
    static void set_t_page_save_settings(int value);
    static uint8_t get_temporary_target_temp();
    static uint8_t get_menu_item();
    static void set_menu_item(uint8_t menu_item);
    
    DisplayManager();
    void display_init();
    void paint();
    void display_off();
    void display_on();
    void display_lightning();
    void set_display_data_config(DisplayDataConfig display_data_config);
    void rotary_encoder_action(uint8_t rotary_state, uint8_t session_boiler_mode);
    void check_page();
};


#endif

/*
  Какие вещи нужны для отображения на дисплее?
  текущая температура
  установленная температура
  устанавливаемая температура
  дата/время

  Ошибки
*/
