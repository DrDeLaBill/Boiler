#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <Arduino.h>
#include "BoilerConstants.h"
#include <U8g2lib.h> //модифицированная версия

#include "clock_rtc.h"
#include "tprofile.h"
#include "temp.h"
#include "errors.h"
#include "network.h"

#define LED_PIN                     23      // пин подсветки дисплея

#define CANCEL_TIMEOUT              5000    // время возврата на основной экран, если не было активности, мс
#define SAVE_TIMEOUT                1300    // время показа экрана "сохранено", мс

#define TIMEOUT_LIGHTNING           20000   // время до автоматического выключения подсветки

#define REDRAW_TIMEOUT              5000    // максимальное время до обновления дисплея

enum DisplayPages {
  pageTemp,
  pageTempSet,
  pageSaveSettings,
  pageSettings,
  pageSetMode,
  pageError,
  pageResetSettings
};

class DisplayManager
{
  private:
    DisplayPages page;
    bool redraw_display = false;            // флаг перерисовки дисплея
    uint8_t menu_item = 0;                  // позиция рамки в меню
    uint32_t t_page_save_settings = 0;        // отображать "Сохранено" на 1с
    uint32_t t_newPage = 0;                 // если в течении 5с не было изменений, то отмена TODO: почему эта переменная ещё и extern
    uint32_t last_time_redraw = 0;          //
    uint8_t temporary_target_temp;          // настраиваемая температура
    
    //U8G2_PCD8544_84X48_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 14, /* data=*/ 27, /* cs=*/ 26, /* dc=*/ 25, /* reset=*/ 33);  // Nokia 5110 Display
    U8G2_PCD8544_84X48_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 25, /* dc=*/ 26, /* reset=*/ U8X8_PIN_NONE);  // Nokia 5110 Display
  public:
    DisplayManager();
    void display_init();
    void paint();
    void display_off();
    void display_on();
    void display_lightning();
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
