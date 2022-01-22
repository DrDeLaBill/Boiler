#include "PumpManager.h"

PumpManager::PumpManager() {
  this->pump_off_delay = 0;
  this->pump_init();
}

void PumpManager::pump_init(){
  pinMode(PUMP_OUT_PIN, OUTPUT);
  pump_off();
}

void PumpManager::pump_on(){
  digitalWrite(PUMP_OUT_PIN, HIGH);
  this->pump_state = PUMP_ON;
}

void PumpManager::pump_off(){
  if (this->pump_state == PUMP_ON){
    this->pump_state = PUMP_WAIT;
    this->pump_off_delay = millis() + PUMP_OFF_TIMEOUT;
  } else if (this->pump_state == PUMP_WAIT){
    if (millis() >= this->pump_off_delay){
      digitalWrite(PUMP_OUT_PIN, LOW);
      this->pump_state = PUMP_OFF;
    }
  }
}

uint8_t PumpManager::check_pump(){
  if (this->pump_state == PUMP_ON){
    // если датчик протока не работает, то ошибка.
    if (digitalRead(FLOW_IN_PIN) == HIGH){
      delay(5);
      if (digitalRead(FLOW_IN_PIN) == HIGH){
        // если уровень высокий, то протока нет.
        Serial.println(F("ERROR: PUMP is BROKEN"));
        return ERROR_PUMPBROKEN;
      }
    }
  }
  return ERROR_NOERROR;
}
