#include "EncoderManager.h"

//TODO: extern
extern BoilerConfig BoilerCfg;
extern uint8_t user_boiler_mode;
extern DisplayPages page;
extern bool redraw_display;                       // флаг для перерисовки дисплея
extern uint8_t menu_item;
extern uint8_t temporary_target_temp;
extern uint8_t current_temp;
extern uint32_t t_pageSaveSettings;
extern uint32_t t_newPage;

EncoderManager::EncoderManager() {
  this->last_time_button = 0;
  this->last_time_debounce = 0;
  this->button_last_state = BUTTON_NO_PRESSED;
  this->encoder_init();
}

void encoder_init() {
  pinMode(PIN_ENC_BUTTON, INPUT);
  ESP32Encoder::useInternalWeakPullResistors = UP;

  // Attache pins for use as encoder pins
  this->encoder.attachHalfQuad(PIN_ENC_1, PIN_ENC_2);

  // set starting count value after attaching
  this->encoder.setCount(this->count_default);
  this->count = this->count_default;

  gpio_set_pull_mode((gpio_num_t)PIN_ENC_BUTTON, GPIO_PULLUP_ONLY);
  attachInterrupt(PIN_ENC_BUTTON, buttonISR, FALLING);
}

void buttonISR() {
  if (last_time_debounce == 0)
    last_time_debounce = millis();
}

int32_t encoder_get_ticks() {
  int32_t enc = (encoder.getCount() - count) / 2;
  if (enc != 0) {
    count += enc * 2;
  }
  return enc;
}

uint8_t check_encoder_button() {
  /*
     если кнопка нажата, то запоминаем время для антидребезга.
     Смотрим, прошло ли время антидребезга. Проверяем текущее состояние кнопки.
     Если кнопка нажата, то запоминаем время длительности нажатия.
     Если кнопка нажата и время длительности было сохранено - измеряем время длительности.
     Если долгое нажатие - то запоминаем это и возвращаем статус.
     Если кнопка отпущена, то снова запоминаем время антидребезга.
  */
  // возвращает состояние кнопки: кратковременное нажатие, длинное нажатие
  if (last_time_debounce == 0) {
    return BUTTON_NO_PRESSED;
  } else {
    if (millis() - last_time_debounce >= TIME_DEBOUNCE) {
      // если кнопка нажата и время дребезга прошло проверяем на шум.
      if (digitalRead(PIN_ENC_BUTTON) == LOW) {
        // если кнопка нажата, то запоминаем время, если еще не сделали
        if (last_time_button == 0 && button_last_state != BUTTON_HOLDED) {
          last_time_button = millis();
        } else {
          // оценим длительность нажатия
          if (millis() - last_time_button >= TIME_BUTTON_HOLDED && button_last_state != BUTTON_HOLDED) {
            button_last_state = BUTTON_HOLDED;
            last_time_button = 0;
            return BUTTON_HOLDED;
          }
        }
      } else {
        // если кнопка отпущена, то подождем время дребезга
        if (last_time_button != 0) {
          // оценим время нажатия кнопки
          last_time_debounce = millis();
          if (millis() - last_time_button >= TIME_BUTTON_PRESSED && button_last_state != BUTTON_HOLDED) {
            last_time_button = 0;
            button_last_state = BUTTON_PRESSED;
            return BUTTON_PRESSED;
          }
        } else {
          // кнопка совсем отпущена и время дребезга закончилось
          last_time_debounce = 0;
          button_last_state = BUTTON_RELEASED;
        }
      }
    }
    return BUTTON_NO_PRESSED;
  }
}

uint8_t check_encoder(bool standby) {
  // в зависимости от текущего режима котла обработаем энкодер с кнопкой

  uint8_t button_state = check_encoder_button();

  if (button_state == BUTTON_HOLDED) {
    // длительное нажатие кнопки

    #ifdef DEBUG_ON
    Serial.println("button holded");
    #endif
    return BUTTON_HOLDED;
  }

  if (!standby) {
    // если котел в рабочем режиме, то обрабатываем действия пользователя
    int32_t ticks = encoder_get_ticks();

    if (button_state == BUTTON_PRESSED) {
      // если кнопка была нажата

      button_pressed();
      button_state = BUTTON_NO_PRESSED;
    }

    // если было вращение энкодера
    if (ticks > 0) {
      // вращение вправо

      rotary_right();
    } else if (ticks < 0) {
      // вращение влево

      rotary_left();
    }
  }
}

void button_pressed() {
  // пробежимся по меню и сделаем соответствующие изменения
  t_newPage = millis();
  redraw_display = true;

  switch (page) {
    case pageTemp:
      // переходим из основного окна в режим настройки температуры

      page = pageTempSet;
      temporary_target_temp = get_target_temp();
      break;

    case pageTempSet:
      // сохраняем установленную температуру

      page = pageSaveSettings;
      t_pageSaveSettings = millis();
      set_target_temp(temporary_target_temp);
      break;

    case pageSettings:
      // переход в подменю настроек

      switch (menu_item) {
        case 0:
          // включаем рамку выбора

          menu_item = 1;
          break;

        case 1:
          // переходим в выбор текущего режима работы

          page = pageSetMode;
          if (user_boiler_mode == MODE_PROFILE){
            menu_item = 1;
          } else if (user_boiler_mode == MODE_WATER){
            menu_item = 2;
          } else if (user_boiler_mode == MODE_AIR){
            menu_item = 3;
          }
          break;

        case 2:
          // стираем ее_пром
          
          page = pageResetSettings;
          t_pageSaveSettings = millis();
          clearEeprom();
          break;

        default:
          break;
      }

      break;

    case pageSetMode:
      // страница выбора режима работы

      if (menu_item == 1) {
        // если выбран термопрофиль
        set_boiler_mode(MODE_PROFILE);
      } else if (menu_item == 2){
        // если выбран внутренний датчик
        set_boiler_mode(MODE_WATER);
      } else if (menu_item == 3){
        // если выбран внешний датчик
        set_boiler_mode(MODE_AIR);
      }
      page = pageSaveSettings;
      t_pageSaveSettings = millis();
      break;

    default:
      break;
  }
}

void rotary_right() {
  // обработаем вращение энкодера вправо
  t_newPage = millis();
  redraw_display = true;

  switch (page) {
    case pageTemp:
      // страница с настройками

      page = pageSettings;
      break;

    case pageTempSet:
      // страница настройки устанавливаемой температуры

      if (user_boiler_mode == MODE_WATER){
        if (temporary_target_temp < WATER_TEMP_MAX) {
          temporary_target_temp++;
        }
      } else {
        if (temporary_target_temp < AIR_TEMP_MAX) {
          temporary_target_temp++;
        }
      }
      break;

    case pageSettings:
      // страница настроек

      if (menu_item != 0 && menu_item < 3) {
        menu_item++;
      }
      break;

    case pageSetMode:
      // страница выбора режима работы

      if (menu_item < 3) {
        menu_item++;
      }
      break;

    default:
      break;
  }
}

void rotary_left() {
  // обработаем вращение энкодера влево
  t_newPage = millis();
  redraw_display = true;

  switch (page) {
    case pageTempSet:
      // страница настройки устанавливаемой температуры

      if (user_boiler_mode == MODE_WATER){
        if (temporary_target_temp > WATER_TEMP_MIN) {
          temporary_target_temp--;
        }
      } else {
        if (temporary_target_temp > AIR_TEMP_MIN) {
          temporary_target_temp--;
        }
      }
      break;

    case pageSettings:
      // страница настроек

      if (menu_item == 0) {
        page = pageTemp;
      } else {
        if (menu_item > 1) {
          menu_item--;
        }
      }
      break;

    case pageSetMode:
      // страница выбора режима работы

      if (menu_item > 1) {
        menu_item--;
      }
      break;

    default:
      break;
  }
}
