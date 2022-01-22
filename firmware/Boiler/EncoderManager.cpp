#include "EncoderManager.h"

EncoderManager::EncoderManager() {
  this->encoder = new ESP32Encoder();
  this->last_time_button = 0;
  EncoderManager::last_time_debounce = 0;
  this->button_last_state = BUTTON_NO_PRESSED;
  this->button_rotary_state = BUTTON_NON_ROTARY;
  this->encoder_init();
}

void EncoderManager::encoder_init() {
  pinMode(PIN_ENC_BUTTON, INPUT);
  ESP32Encoder::useInternalWeakPullResistors = UP;

  // Attache pins for use as encoder pins
  this->encoder->attachHalfQuad(PIN_ENC_1, PIN_ENC_2);

  // set starting count value after attaching
  this->encoder->setCount(this->count_default);
  this->count = this->count_default;

  gpio_set_pull_mode((gpio_num_t)PIN_ENC_BUTTON, GPIO_PULLUP_ONLY);
  attachInterrupt(PIN_ENC_BUTTON, EncoderManager::buttonISR, FALLING);
}

void EncoderManager::buttonISR() {
  if (EncoderManager::last_time_debounce == 0)
    EncoderManager::last_time_debounce = millis();
}

int32_t EncoderManager::encoder_get_ticks() {
  int32_t enc = (this->encoder->getCount() - this->count) / 2;
  if (enc != 0) {
    this->count += enc * 2;
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
  if (this->last_time_debounce == 0) {
    return BUTTON_NO_PRESSED;
  } else {
    if (millis() - this->last_time_debounce >= TIME_DEBOUNCE) {
      // если кнопка нажата и время дребезга прошло проверяем на шум.
      if (digitalRead(PIN_ENC_BUTTON) == LOW) {
        // если кнопка нажата, то запоминаем время, если еще не сделали
        if (this->last_time_button == 0 && this->button_last_state != BUTTON_HOLDED) {
          this->last_time_button = millis();
        } else {
          // оценим длительность нажатия
          if (millis() - this->last_time_button >= TIME_BUTTON_HOLDED && this->button_last_state != BUTTON_HOLDED) {
            this->button_last_state = BUTTON_HOLDED;
            this->last_time_button = 0;
            return BUTTON_HOLDED;
          }
        }
      } else {
        // если кнопка отпущена, то подождем время дребезга
        if (this->last_time_button != 0) {
          // оценим время нажатия кнопки
          this->last_time_debounce = millis();
          if (millis() - this->last_time_button >= TIME_BUTTON_PRESSED && this->button_last_state != BUTTON_HOLDED) {
            this->last_time_button = 0;
            this->button_last_state = BUTTON_PRESSED;
            return BUTTON_PRESSED;
          }
        } else {
          // кнопка совсем отпущена и время дребезга закончилось
          this->last_time_debounce = 0;
          this->button_last_state = BUTTON_RELEASED;
        }
      }
    }
    return BUTTON_NO_PRESSED;
  }
}

uint8_t EncoderManager::check_encoder(bool standby) {
  // в зависимости от текущего режима котла обработаем энкодер с кнопкой

  uint8_t button_state = this->check_encoder_button();

  if (button_state == BUTTON_HOLDED) {
    // длительное нажатие кнопки

    #ifdef DEBUG_ON
    Serial.println("button holded");
    #endif
    return BUTTON_HOLDED;
  }

  if (!standby) {
    // если котел в рабочем режиме, то обрабатываем действия пользователя
    int32_t ticks = this->encoder_get_ticks();

    if (button_state == BUTTON_PRESSED) {
      // если кнопка была нажата

      //TODO :перенести функцию buutton_pressed со свитч в boiler controller (возвращать work_mode (наверно))
      this->button_pressed();
      button_state = BUTTON_NO_PRESSED;
    }

    // если было вращение энкодера
    if (ticks > 0) {
      // вращение вправо
      this->button_rotary_state = BUTTON_ROTARY_RIGHT;
      this->rotary_right();
    } else if (ticks < 0) {
      // вращение влево
      this->button_rotary_state = BUTTON_ROTARY_LEFT;
      this->rotary_left();
    } else {
      this->button_rotary_state = BUTTON_NON_ROTARY;
    }
  }
  
  return button_state;
}

bool EncoderManager::is_button_holded(uint8_t work_mode) {
  return this->check_encoder(work_mode) == BUTTON_HOLDED;
}

uint8_t EncoderManager::get_button_rotary_state() {
  return this->button_rotary_state;
}
