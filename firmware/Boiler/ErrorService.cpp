#include "ErrorService.h"

//TODO: extern
extern DisplayPages page;
extern bool redraw_display;

ErrorService::ErrorService() {
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
  Serial.print(F("SSR_value: "));
  Serial.println(analogRead(SSR_IN_PIN));

  ершы-Юuser_error = ERROR_NOERROR;
}

void ErrorService::check_failure(){
  // проверяем систему на появление аварийных ситуаций
  //TODO: what?
  if (check_pump() != ERROR_NOERROR){
    page = pageError;
    user_error = ERROR_PUMPBROKEN;
    redraw_display = true;
    return;
  }
  if (user_error == ERROR_PUMPBROKEN || user_error == ERROR_OVERHEAT){
    // сбрасываем текущие ошибки, если они были и перестали быть.
    user_error = ERROR_NOERROR;
    page = pageTemp;
    this->_clear_errors();
  }
}

void ErrorService::get_errors_list(uint8_t *result_errors_list) {
  // Проверка на соответствие результирующего массива по его размеру
  if (sizeof(result_errors_list) < ERRORS_COUNT) {
    Serial.println(F("ERROR: out of range array result_errors_list, please check the code"));
    Serial.println(F("ERROR: error display will not work"));
    return;
  }
  for(int i = 0; i < this->errors_list.size(); i++) 
  {
     result_errors_list[i] = this->errors_list[i];
  }
}

void ErrorService::_add_error(uint8_t new_error) {
  this->errors_list.push_back(new_error);
}

void ErrorService::_remove_error() {
  
}

void ErrorService::_clear_errors() {
  this->errors_list.clear();
}

bool is_set_error(uint8_t error_name) {
  for (int i = 0; i < this->errors_list.size(); i++) {
    if (this->errors_list.at(i) == error_name) {
      return true;
    }
  }
  return false;
}
