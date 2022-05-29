#include "PumpManager.h"

uint8_t PumpManager::pump_state = PUMP_OFF;
uint32_t PumpManager::pump_off_delay = 0;

PumpManager::PumpManager() {
  pinMode(PUMP_OUT_PIN, OUTPUT);
  PumpManager::pump_off();
  Serial.println(F("Pump manager start"));
}

void PumpManager::pump_on(){
  digitalWrite(PUMP_OUT_PIN, HIGH);
  PumpManager::pump_state = PUMP_ON;
}

void PumpManager::pump_off(){
  if (PumpManager::pump_state == PUMP_ON){
    PumpManager::pump_state = PUMP_WAIT;
    PumpManager::pump_off_delay = millis() + PUMP_OFF_TIMEOUT;
  } else if (PumpManager::pump_state == PUMP_WAIT){
    if (millis() >= PumpManager::pump_off_delay){
      digitalWrite(PUMP_OUT_PIN, LOW);
      PumpManager::pump_state = PUMP_OFF;
    }
  }
}

bool PumpManager::is_pump_broken(){
  if (PumpManager::pump_state == PUMP_ON){
    // если датчик протока не работает, то ошибка.
    if (digitalRead(FLOW_IN_PIN) == HIGH){
      delay(5);
      if (digitalRead(FLOW_IN_PIN) == HIGH){
        // если уровень высокий, то протока нет.
        Serial.println(F("ERROR: PUMP is BROKEN"));
        return true;
      }
    }
  }
  return false;
}
