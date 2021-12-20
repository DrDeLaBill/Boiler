#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <Arduino.h>

#include "clock_rtc.h"
#include "tprofile.h"
#include "temp.h"
#include "errors.h"
#include "network.h"

#define LED_PIN               23      // пин подсветки дисплея

#define CANCEL_TIMEOUT        5000    // время возврата на основной экран, если не было активности, мс
#define SAVE_TIMEOUT          1300    // время показа экрана "сохранено", мс

#define TIMEOUT_LIGHTNING     20000   // время до автоматического выключения подсветки

#define REDRAW_TIMEOUT        5000    // максимальное время до обновления дисплея

/*
  Какие вещи нужны для отображения на дисплее?
  текущая температура
  установленная температура
  устанавливаемая температура
  дата/время

  Ошибки

*/


enum DisplayPages {
  pageTemp,
  pageTempSet,
  pageSaveSettings,
  pageSettings,
  pageSetMode,
  pageError,
  pageResetSettings
};



void display_init();
void paint();
void display_off();
void display_on();
void display_lightning();





#endif
