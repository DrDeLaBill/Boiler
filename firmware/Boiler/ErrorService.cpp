#include "ErrorService.h"

uint8_t ErrorService::errors_list[ERRORS_COUNT] = {};

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
  Serial.print(F("SSR input pin: "));
  Serial.println(analogRead(SSR_IN_PIN));
  Serial.println(F("Error service start"));
}

void ErrorService::check_failure() {
  // проверяем систему на появление аварийных ситуаций
  if (PumpManager::is_pump_broken()) {
    Serial.println(F("Check failure: "));
    ErrorService::add_error(ERROR_PUMPBROKEN);
  } else {
    // сбрасываем текущие ошибки
    // ErrorService::clear_errors();
  }
}

void ErrorService::get_errors_list(uint8_t result_errors_list[]) {
  // Проверка на соответствие результирующего массива по его размеру
  if (sizeof(result_errors_list) > ERRORS_COUNT) {
    Serial.println(F("ERROR: out of range array result_errors_list, please check configuration"));
    Serial.println(F("ERROR: error display will not work"));
    return;
  }
  for (uint8_t i = 0; i < ERRORS_COUNT; i++)
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
    for (uint8_t i = 0; i < ERRORS_COUNT; i++) {
      if (errors_list[i] == 0) {
        errors_list[i] = new_error;
        return;
      }
    }
  }
}

void ErrorService::clear_errors() {
  Serial.println(F("Clear errors"));
  for (uint8_t i = 0; i < ERRORS_COUNT; i++) {
    ErrorService::errors_list[i] = 0;
  }
  DisplayManager::set_page_name(pageTemp);
}

bool ErrorService::is_set_error(uint8_t error_name) {
  for (auto error : ErrorService::errors_list) {
    if (error == error_name) {
      return true;
    }
  }
  return false;
}

bool ErrorService::is_no_errors() {
  return ErrorService::errors_list[0] == ERROR_NOERROR;
}

bool ErrorService::type_error_validate(uint8_t error_type) {
  return 0 < error_type && error_type < ERRORS_COUNT;
}

void ErrorService::init_error_actions() {
  if (ErrorService::is_set_error(ERROR_TEMPSENSBROKEN)){
    DisplayManager::set_page_name(pageError);
    ErrorService::enable_crash_out_pin();
  } else if (ErrorService::is_set_error(ERROR_PUMPBROKEN)) {
    ErrorService::enable_crash_out_pin();
  } else if (ErrorService::is_no_errors()) {
    ErrorService::disable_crash_out_pin();
  }
}

void ErrorService::enable_crash_out_pin() {
  digitalWrite(CRASH_OUT_PIN, HIGH);
}

void ErrorService::disable_crash_out_pin() {
  digitalWrite(CRASH_OUT_PIN, LOW);
}

bool ErrorService::if_single_error(uint8_t error_name) {
  if (error_name == ERROR_NOERROR) {
    return ErrorService::errors_list[1] == 0;
  }
  return ErrorService::errors_list[0] == error_name && ErrorService::errors_list[1] == 0;
}
