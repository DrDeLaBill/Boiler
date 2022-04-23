#include "EncoderManager.h"

ESP32Encoder EncoderManager::encoder;
const int32_t EncoderManager::count_default = 1000000;
int32_t EncoderManager::count = 0;
uint32_t EncoderManager::last_time_button = 0;
uint8_t EncoderManager::button_last_state = BUTTON_NO_PRESSED;
uint8_t EncoderManager::button_rotary_state = BUTTON_NON_ROTARY;
uint32_t EncoderManager::last_time_debounce = 0;
  
EncoderManager::EncoderManager() {
  pinMode(PIN_ENC_BUTTON, INPUT);
  ESP32Encoder::useInternalWeakPullResistors = UP;
  // Attache pins for use as encoder pins
  EncoderManager::encoder.attachHalfQuad(PIN_ENC_1, PIN_ENC_2);
  // set starting count value after attaching
  EncoderManager::encoder.setCount(EncoderManager::count_default);
  EncoderManager::count = EncoderManager::count_default;
  gpio_set_pull_mode((gpio_num_t)PIN_ENC_BUTTON, GPIO_PULLUP_ONLY);
  attachInterrupt(PIN_ENC_BUTTON, EncoderManager::buttonISR, FALLING);
  Serial.println(F("Encoder manager start"));
}

void EncoderManager::buttonISR() {
  if (EncoderManager::last_time_debounce == 0)
    EncoderManager::last_time_debounce = millis();
}

int32_t EncoderManager::encoder_get_ticks() {
  int32_t enc = (EncoderManager::encoder.getCount() - EncoderManager::count) / 2;
  if (enc != 0) {
    EncoderManager::count += enc * 2;
  }
  return enc;
}

uint8_t EncoderManager::check_encoder_button() {
  /*
     если кнопка нажата, то запоминаем время для антидребезга.
     Смотрим, прошло ли время антидребезга. Проверяем текущее состояние кнопки.
     Если кнопка нажата, то запоминаем время длительности нажатия.
     Если кнопка нажата и время длительности было сохранено - измеряем время длительности.
     Если долгое нажатие - то запоминаем это и возвращаем статус.
     Если кнопка отпущена, то снова запоминаем время антидребезга.
  */
  // возвращает состояние кнопки: кратковременное нажатие, длинное нажатие
  // возвращает состояние кнопки: кратковременное нажатие, длинное нажатие
  if (EncoderManager::last_time_debounce == 0) {
    return BUTTON_NO_PRESSED;
  } else {
    if (abs(millis() - EncoderManager::last_time_debounce) >= TIME_DEBOUNCE) {
      // если кнопка нажата и время дребезга прошло проверяем на шум.
      if (digitalRead(PIN_ENC_BUTTON) == LOW) {
        // если кнопка нажата, то запоминаем время, если еще не сделали
        if (EncoderManager::last_time_button == 0 && EncoderManager::button_last_state != BUTTON_HOLDED) {
          EncoderManager::last_time_button = millis();
        } else {
          // оценим длительность нажатия
          if (abs(millis() - EncoderManager::last_time_button) >= TIME_BUTTON_HOLDED && EncoderManager::button_last_state != BUTTON_HOLDED) {
            EncoderManager::button_last_state = BUTTON_HOLDED;
            EncoderManager::last_time_button = 0;
            return BUTTON_HOLDED;
          }
        }
      } else {
        // если кнопка отпущена, то подождем время дребезга
        if (EncoderManager::last_time_button != 0) {
          // оценим время нажатия кнопки
          EncoderManager::last_time_debounce = millis();
          if (abs(millis() - last_time_button) >= TIME_BUTTON_PRESSED && EncoderManager::button_last_state != BUTTON_HOLDED) {
            EncoderManager::last_time_button = 0;
            EncoderManager::button_last_state = BUTTON_PRESSED;
            return BUTTON_PRESSED;
          }
        } else {
          // кнопка совсем отпущена и время дребезга закончилось
          EncoderManager::last_time_debounce = 0;
          EncoderManager::button_last_state = BUTTON_RELEASED;
        }
      }
    }
    return BUTTON_NO_PRESSED;
  }
}

uint8_t EncoderManager::check_encoder(bool is_standby_mode) {
  // в зависимости от текущего режима котла обработаем энкодер с кнопкой

  uint8_t button_state = EncoderManager::check_encoder_button();

  if (button_state == BUTTON_HOLDED) {
    // длительное нажатие кнопки
    Serial.println("Button holded");
    return BUTTON_HOLDED;
  }

  if (!is_standby_mode) {
    // если котел в рабочем режиме, то обрабатываем действия пользователя
    int32_t ticks = EncoderManager::encoder_get_ticks();

    if (button_state == BUTTON_PRESSED) {
      // если кнопка была нажата
      EncoderManager::button_pressed_action();
      button_state = BUTTON_NO_PRESSED;
    }

    // если было вращение энкодера
    if (ticks > 0) {
      // вращение вправо
      DisplayManager::rotary_right();
    } else if (ticks < 0) {
      // вращение влево
      DisplayManager::rotary_left();
    }
  }

  return BUTTON_NO_PRESSED;
}

void EncoderManager::button_pressed_action() {
  // пробежимся по меню и сделаем соответствующие изменения
  DisplayManager::set_t_newPage(millis());

  switch (DisplayManager::get_page_name()) {
    case pageTemp:
      // переходим из основного окна в режим настройки температуры
      DisplayManager::set_page_name(pageTempSet);
      DisplayManager::set_temporary_target_temp(
        BoilerProfile::get_target_temp()
      );
      break;

    case pageTempSet:
      // сохраняем установленную температуру
      DisplayManager::set_page_name(pageSaveSettings);
      DisplayManager::set_t_page_save_settings(millis());
      BoilerProfile::set_target_temp(
        DisplayManager::get_temporary_target_temp()
      );
      break;

    case pageSettings:
      // переход в подменю настроек

      switch (DisplayManager::get_menu_item()) {
        case 0:
          // включаем рамку выбора
          DisplayManager::set_menu_item(1);
          break;

        case 1:
          // переходим в выбор текущего режима работы
          DisplayManager::set_page_name(pageSetMode);
          if (BoilerProfile::is_set_session_boiler_mode(MODE_PROFILE)) {
            DisplayManager::set_menu_item(1);
          } else if (BoilerProfile::is_set_session_boiler_mode(MODE_WATER)) {
            DisplayManager::set_menu_item(2);
          } else if (BoilerProfile::is_set_session_boiler_mode(MODE_AIR)) {
            DisplayManager::set_menu_item(3);
          }
          break;

        case 2:
          // стираем ее_пром
          DisplayManager::set_page_name(pageResetSettings);
          DisplayManager::set_t_page_save_settings(millis());
          BoilerProfile::clear_eeprom();
          break;
        default:
          break;
      }

      break;

    case pageSetMode:
      // страница выбора режима работы

      if (DisplayManager::get_menu_item() == 1) {
        // если выбран термопрофиль
        BoilerProfile::set_boiler_mode(MODE_PROFILE);
      } else if (DisplayManager::get_menu_item() == 2) {
        // если выбран внутренний датчик
        BoilerProfile::set_boiler_mode(MODE_WATER);
      } else if (DisplayManager::get_menu_item() == 3) {
        // если выбран внешний датчик
        BoilerProfile::set_boiler_mode(MODE_AIR);
      }
      DisplayManager::set_page_name(pageResetSettings);
      DisplayManager::set_t_page_save_settings(millis());
      break;

    default:
      break;
  }
}


bool EncoderManager::is_button_holded(uint8_t work_mode) {
  return EncoderManager::check_encoder(work_mode) == BUTTON_HOLDED;
}

uint8_t EncoderManager::get_button_rotary_state() {
  return EncoderManager::button_rotary_state;
}
