#include "display.h"

#include <Arduino.h>
#include <U8g2lib.h> //модифицированная версия
//#include "rus5x8.h"
//#include "rus6x10.h"
//#include <SPI.h>

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

DisplayPages page;

bool redraw_display = false;            // флаг перерисовки дисплея
uint8_t menu_item = 0;                  // позиция рамки в меню
uint32_t t_pageSaveSettings = 0;        // отображать "Сохранено" на 1с
uint32_t t_newPage = 0;                 // если в течении 5с не было изменений, то отмена
uint32_t last_time_redraw = 0;          //
uint8_t temporary_target_temp;          // настраиваемая температура

extern BoilerConfig BoilerCfg;

extern uint8_t current_temp;            // текущая температура для отображения и ПИД
extern uint32_t t_newPage;
extern uint8_t user_boiler_mode;
extern uint8_t user_error;
extern bool connected_to_server;
extern uint8_t radio_connected;

//U8G2_PCD8544_84X48_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 14, /* data=*/ 27, /* cs=*/ 26, /* dc=*/ 25, /* reset=*/ 33);  // Nokia 5110 Display
U8G2_PCD8544_84X48_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 25, /* dc=*/ 26, /* reset=*/ U8X8_PIN_NONE);  // Nokia 5110 Display


void display_init() {
  u8g2.begin();
  u8g2.setContrast(70);
  u8g2.enableUTF8Print();
  u8g2.setFontDirection(0);
  page = pageTemp;
  redraw_display = true;
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  t_newPage = millis();
}

void display_on() {
  // включаем дисплей и подсветку. Переходим на основную страницу.
  page = pageTemp;
  menu_item = 0;
  redraw_display = true;
  u8g2.setPowerSave(false);
  digitalWrite(LED_PIN, HIGH);
  t_newPage = millis();
}

void display_off() {
  // выключаем дисплей и подсветку
  u8g2.setPowerSave(true);
  digitalWrite(LED_PIN, LOW);
}

void display_lightning() {
  // плавно выключаем подсветку при отсутствии активности
  static uint8_t brightness = 255;
  if (t_newPage != 0) {
    // если недавно была активность то ждем время до выключения
    if (millis() - t_newPage >= TIMEOUT_LIGHTNING) {
      ledcWrite(0, brightness--);
      if (brightness == 0) {
        ledcWrite(0, 0);
        t_newPage = 0;
      }
    } else {
      // если время не подходит, то обновляем данные.
      brightness = 255;
      ledcWrite(0, 255);
    }
  }
}

void check_page() {
  // проверяем время неактивности. Если в течении 5с не было активности возвращаемся на начальный экран.
  // время экрана "сохранено" - 1с

  if (page != pageTemp && page != pageError) {

    if (page == pageSaveSettings || page == pageResetSettings) {
      if (millis() - t_pageSaveSettings >= SAVE_TIMEOUT) {
        page = pageTemp;
        menu_item = 0;
        redraw_display = true;
        return;
      }
    } else if (millis() - t_newPage >= CANCEL_TIMEOUT) {
      page = pageTemp;
      menu_item = 0;
      redraw_display = true;
    }
  }
}

void paint() {
  // отрисовка дисплея

  if (millis() - last_time_redraw >= REDRAW_TIMEOUT) {
    last_time_redraw = millis();
    redraw_display = true;
  }

  check_page();
  //display_lightning();

  if (redraw_display){   // если пора, то...
    last_time_redraw = millis();
    u8g2.clearBuffer();

    switch (page) {
      case pageTemp:
        // основной экран. Отображение текущей и уставной температуры.

        // если котел подключен к WiFi, то показываем значок вайфай
        if (WiFi.status() == WL_CONNECTED){
          u8g2.drawXBM(0, 0, 12, 9, wifi);
        }

        // если у нас сейчас "работает" нагрев, то показываем это.
        if (digitalRead(SSR1_OUT_PIN)){
          u8g2.drawXBM(14, 0, 9, 9, heat);
        }

        // если есть связь с сервером - то показываем это
        if (connected_to_server == CONNECTED){
          u8g2.drawXBM(26, 0, 8, 9, inet);
        }


        // рисование текущего датчика температуры
        if (user_boiler_mode == MODE_AIR || user_boiler_mode == MODE_PROFILE){
          // если выбран внешний датчик температуры или термопрофиль
          u8g2.drawXBM(65, 20, 16, 16, pict_air);
        } else if (user_boiler_mode == MODE_WATER){
          // иначе если выбран датчик температуры теплоносителя
          u8g2.drawXBM(65, 20, 16, 16, pict_water);
        }

        // если есть радиосвязь с датчиком, то рисуем термометр
        if (radio_connected == RADIO_ON){
          u8g2.drawXBM(50, 20, 9, 16, pict_therm);
        }

        // текущая температура
        //u8g2.setFont(u8g2_font_inb30_mr);
        u8g2.setFont(u8g2_font_inb30_mn);
        u8g2.setCursor(0, 40);
        u8g2.print(current_temp); //

        // установленная температура
        u8g2.setFont(u8g2_font_luBS12_tr);
        u8g2.setCursor(43, 12);
        u8g2.print(get_target_temp());
        u8g2.setFont(u8g_font_5x8);
        u8g2.setCursor(67, 5);
        u8g2.print("o");
        u8g2.setFont(u8g2_font_ncenR12_tf);
        u8g2.setCursor(71, 12);
        u8g2.print("C");

        // отрисовка текущей даты и времени
        u8g2.setFont(u8g_font_5x8);
        u8g2.setCursor(0, 48);
        u8g2.print(clock_get_time("d/m/Y"));

        u8g2.setFont(u8g2_font_t0_12b_mr);
        u8g2.drawStr(54, 48, clock_get_time("H:i"));

        break;

      case pageTempSet:
        // страница настройки установленной температуры

        u8g2.setFont(u8g2_font_5x8_t_cyrillic);
        u8g2.setCursor(0, 6);
        u8g2.print("Требуемая");
        u8g2.setCursor(0, 13);
        u8g2.print("температура:");

        u8g2.setCursor(54, 30);
        u8g2.print("o");

        u8g2.setFont(u8g2_font_luBS12_tr);
        u8g2.setCursor(58, 38);
        u8g2.print("C");

        u8g2.setFont(u8g2_font_luBS12_tr);
        u8g2.setCursor(29, 38);
        u8g2.print(temporary_target_temp);
        break;

      case pageSaveSettings:
        // страница сохранения новой установленной температуры

        u8g2.setFont(u8g2_font_5x8_t_cyrillic);
        u8g2.setCursor(18, 22);
        u8g2.print("Сохранено!");  //Страницы на экране
        break;

      case pageResetSettings:
        // страница сохранения новой установленной температуры

        u8g2.setFont(u8g2_font_5x8_t_cyrillic);
        u8g2.setCursor(10, 8);
        u8g2.print("Настройки");
        u8g2.setCursor(10, 20);
        u8g2.print("возвращены к");
        u8g2.setCursor(10, 32);
        u8g2.print("заводским");
        u8g2.setCursor(10, 44);
        u8g2.print("установкам !");

        break;

      case pageSettings:
        // страница настроек котла.
        
        u8g2.setFont(u8g2_font_5x8_t_cyrillic);
        u8g2.setCursor(5, 8);
        u8g2.print("Режим работы");
        u8g2.setCursor(5, 20);
        u8g2.print("Сброс настроек");
        

        if (menu_item == 0) {
          // без выбора
        } else if (menu_item == 1) {
          u8g2.drawRFrame(0,  1, 78, 10, 4);
        } else if (menu_item == 2) {
          u8g2.drawRFrame(0, 12, 78, 10, 4);
        }
        break;

      case pageSetMode:
        // страница выбора режима работы по уставке теплоносителя, воздуха или по термопрофилю

        u8g2.setFont(u8g2_font_5x8_t_cyrillic);
        u8g2.setCursor(5, 8);
        u8g2.print("термопрофиль");
        u8g2.setCursor(5, 20);
        u8g2.print("темп. вода");
        u8g2.setCursor(5, 32);
        u8g2.print("темп. воздух");

        if (menu_item == 1) {
          u8g2.drawRFrame(0, 1, 78, 10, 4);
        } else if (menu_item == 2) {
          u8g2.drawRFrame(0, 12, 78, 10, 4);
        } else if (menu_item == 3){
          u8g2.drawRFrame(0, 23, 78, 10, 4);
        }
        break;

      case pageError:
        // страница отображения критических ошибок

        u8g2.setFont(u8g2_font_6x12_t_cyrillic);
        u8g2.setCursor(18, 12);
        u8g2.print("АВАРИЯ!");

        if (user_error == OVERHEAT || user_error == WATEROVERHEAT) {
          // перегрев

          u8g2.setCursor(10, 32);
          u8g2.print("Перегрев!");
        } else if (user_error == PUMPBROKEN) {
          // датчик протока сигнализирует об отсутствии потока

          u8g2.setCursor(10, 32);
          u8g2.print("Нет");
          u8g2.setCursor(10, 44);
          u8g2.print("протока!");
        } else if (user_error == SSRBROKEN) {
          // твердотельные реле неисправны

          u8g2.setCursor(10, 32);
          u8g2.print("Перегрев");
          u8g2.setCursor(10, 44);
          u8g2.print("ТТ реле!");
        } else if (user_error == TEMPSENSBROKEN) {
          // датчик температуры теплоносителя неисправен

          u8g2.setCursor(10, 32);
          u8g2.print("Датчик темп.");
          u8g2.setCursor(10, 44);
          u8g2.print("неисправен!");
        } else if (user_error == NOPOWER){
          // нет напряжения после ТТР

          u8g2.setCursor(10, 32);
          u8g2.print("Нет");
          u8g2.setCursor(10, 44);
          u8g2.print("нагрева!");
        }
        break;

      default:
        break;
    }

    u8g2.sendBuffer();
    redraw_display = false;
  }
}
