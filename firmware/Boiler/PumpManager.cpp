#include "PumpManager.h"

PumpManager::PumpManager() {
  this->pump_off_delay = 0;
  this->pump_init();
}

void pump_init(){
  pinMode(PUMP_OUT_PIN, OUTPUT);
  pump_off();
}

void pump_on(){
  digitalWrite(PUMP_OUT_PIN, HIGH);
  pump_state = PUMP_ON;
}

void pump_off(){
  if (pump_state == PUMP_ON){
    pump_state = PUMP_WAIT;
    pump_off_delay = millis() + PUMP_OFF_TIMEOUT;
  } else if (pump_state == PUMP_WAIT){
    if (millis() >= pump_off_delay){
      digitalWrite(PUMP_OUT_PIN, LOW);
      pump_state = PUMP_OFF;
    }
  }
}

uint8_t check_pump(){
  if (pump_state == PUMP_ON){
    // если датчик протока не работает, то ошибка.
    if (digitalRead(FLOW_IN_PIN) == HIGH){
      delay(5);
      if (digitalRead(FLOW_IN_PIN) == HIGH){
        // если уровень высокий, то протока нет.
        Serial.println("ERROR: PUMP is BROKEN");
        return PUMP_BROKEN;
      }
    }
  }
  return 0;
}
