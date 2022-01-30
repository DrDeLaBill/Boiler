#include "ErrorService.h"

Vector<uint8_t> ErrorService::errors_list;

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

  this->user_error = ERROR_NOERROR;
}

void ErrorService::check_failure(){
  // проверяем систему на появление аварийных ситуаций
  if (ErrorService::is_set_error(ERROR_SSRBROKEN)){ // check_pump()
    this->user_error = ERROR_PUMPBROKEN;
    return;
  }
  
  // сбрасываем текущие ошибки
  this->user_error = ERROR_NOERROR;
  this->_clear_errors();
}

void ErrorService::get_errors_list(uint8_t result_errors_list[ERRORS_COUNT]) {
  // Проверка на соответствие результирующего массива по его размеру
  if (sizeof(result_errors_list) > ERRORS_COUNT) {
    Serial.println(F("ERROR: out of range array result_errors_list, please check the code"));
    Serial.println(F("ERROR: error display will not work"));
    return;
  }
  for(int i = 0; i < ErrorService::errors_list.size(); i++) 
  {
     result_errors_list[i] = ErrorService::errors_list[i];
  }
}

void ErrorService::add_error(uint8_t new_error) {
  if (ErrorService::is_set_error(new_error)) {
    return;
  } else if (ErrorService::type_error_validate(new_error)) {
    Serial.print(F("ERROR: code-"));
    Serial.print(new_error);
    Serial.println(F(" added to errors list"));
    ErrorService::errors_list.push_back(new_error);
  }
}

void ErrorService::_remove_error() {
  
}

void ErrorService::_clear_errors() {
  ErrorService::errors_list.clear();
}

bool ErrorService::is_set_error(uint8_t error_name) {
  for (int i = 0; i < ErrorService::errors_list.size(); i++) {
    if (ErrorService::errors_list.at(i) == error_name) {
      return true;
    }
  }
  return false;
}

//TODO: переделать в перечисление enum
bool ErrorService::type_error_validate(uint8_t error_type) {
  return (error_type == ERROR_OVERHEAT ||
      error_type == ERROR_PUMPBROKEN ||
      error_type == ERROR_SSRBROKEN ||
      error_type == ERROR_TEMPSENSBROKEN ||
      error_type == ERROR_WATEROVERHEAT ||
      error_type == ERROR_NOPOWER
  );
}

void ErrorService::init_error_actions() {
  for (uint8_t i=0; i < ErrorService::errors_list.size(); i++) {
    switch(ErrorService::errors_list.at(i)) {
      case ERROR_NOERROR:
        break;
      case ERROR_PUMPBROKEN:
        this->enable_crash_out_pin();
        break;
      default:
        this->enable_crash_out_pin();
        break;
    }
  }
}

void ErrorService::enable_crash_out_pin() {
  digitalWrite(CRASH_OUT_PIN, HIGH);
}
