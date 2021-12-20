/*
 * 
 */

#include "errors.h"

uint8_t user_error = NOERROR;

extern DisplayPages page;
extern bool redraw_display;

void init_diagnostics(){
  // система диагностики аварийных ситуаций

  pinMode(SSR_IN_PIN, INPUT);
  pinMode(FLOW_IN_PIN, INPUT_PULLUP);
  pinMode(CRASH_OUT_PIN, OUTPUT);
  digitalWrite(CRASH_OUT_PIN, LOW);

  pinMode(SSR1_OUT_PIN, OUTPUT);
  pinMode(SSR2_OUT_PIN, OUTPUT);
  pinMode(SSR3_OUT_PIN, OUTPUT);
  digitalWrite(SSR1_OUT_PIN, LOW);
  digitalWrite(SSR2_OUT_PIN, LOW);
  digitalWrite(SSR3_OUT_PIN, LOW);
  delay(SSR_DELAY);
  Serial.print("SSR_value: ");
  Serial.println(analogRead(SSR_IN_PIN));

  check_failure();
}

void check_ssr_failure(){
  // TODO: написать проверку исправности ТТР при помощи токового трансформатора. 
  // еще надо добавить вычисление потребляемого тока для расчета мощности. 
  
  // проверяем исправность ТТР
  /*  логика:
   * При включении проверяем количество включенных фаз. Сохраняем это значение.
   * Если используется только 1 фаза:
   * Раз в минуту проверяем: если идет нагрев, то ток должен быть. 
   * Если нагрев выключен - тока нет. Соответствующие ошибки.
   * 
   * Если используются все 3 фазы:
   * Раз в N минут делаем проверку. 
   * Выключаем все нагреватели - ждем - проверяем. 
   * Включаем 1 фазу - ждем  проверяем. Выключаем.
   * Включаем 2 фазу - ждем - проверяем. Выключаем.
   * Включаем 3 фазу - ждем - проверяем. Выключаем.
   * Возвращаем как было.
   */
   
}

void check_failure(){
  // проверяем систему на появление аварийных ситуаций
  
  if (check_pump() != NOERROR){
    page = pageError;
    user_error = PUMPBROKEN;
    redraw_display = true;
    return;
  }

  // сбрасываем текущие ошибки, если они были и перестали быть.
  if (user_error == PUMPBROKEN || user_error == OVERHEAT){
    user_error = NOERROR;
    page = pageTemp;
  }
}
