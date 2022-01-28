/*
 * обработка энкодера с кнопкой
 */
#ifndef _ENCODER_H_
#define _ENCODER_H_

#include <Arduino.h>
#include <ESP32Encoder.h>

#include "BoilerConstants.h"
#include "DisplayManager.h"
#include "BoilerProfile.h"

#define PIN_ENC_BUTTON                19          // пин подключения кнопки
#define PIN_ENC_1                     5      
#define PIN_ENC_2                     18          // пины подключения энкодера

#define TIME_BUTTON_PRESSED           100         // время короткого нажатия
#define TIME_BUTTON_HOLDED            1000        // время длительного нажатия
#define TIME_DEBOUNCE                 30          // время антидребезга кнопки

class EncoderManager
{
  private:
    ESP32Encoder *encoder;
    const int32_t count_default = 1000000;
    int32_t count;
    uint32_t last_time_button;
    uint8_t button_last_state;
    uint8_t button_rotary_state;
  public:
    static uint32_t last_time_debounce;
    
    EncoderManager();
    static void buttonISR();
    void encoder_init();
    int32_t encoder_get_ticks();
    uint8_t check_encoder_button();
    uint8_t check_encoder(bool standby);
    bool is_button_holded(uint8_t work_mode);
    void rotary_right();
    void rotary_left();
    uint8_t get_button_rotary_state();
    void button_pressed_action();
};

#endif
