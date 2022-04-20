#include "RadioSensor.h"

float RadioSensor::radio_sens_temp = 0.0;
uint32_t RadioSensor::last_time_online = 0;
RF24 RadioSensor::radio(PIN_CE, PIN_CSN);

RadioSensor::RadioSensor() {
  // Инициализация модуля NRF24L01
  bool radio_init_state = RadioSensor::radio.begin();
  RadioSensor::radio.setChannel(0x6f);
  RadioSensor::radio.setDataRate(RF24_1MBPS); // Скорость обмена данными 1 Мбит/сек
  RadioSensor::radio.setPALevel(RF24_PA_HIGH);           //
  RadioSensor::radio.openReadingPipe(1, 0x7878787878LL); // Открываем трубу ID передатчика
  RadioSensor::radio.startListening(); // Начинаем прослушивать открываемую трубу
  RadioSensor::clear_timeout_radio_sens();
  if (radio_init_state) {
    Serial.println("Radio module init ok");
  }
  else {
    Serial.println("Radio module init error"); 
  }
}

uint8_t RadioSensor::update_radio_temp(){
	if (millis() - RadioSensor::last_time_online < RECEIVE_TIMEOUT){
		if (RadioSensor::radio.available()){
			// TODO: написать проверку приходящих данных. !! валидация
			// Возможно, добавится отправка других данных с датчика.
			RadioSensor::last_time_online = millis();
			RadioSensor::radio.read(&RadioSensor::radio_sens_temp, sizeof(float));
		  return GOT_EXT_TEMP;
		} else {
		  return NO_EXT_TEMP;
		}
	} else {
		// time over
		RadioSensor::last_time_online = millis() - (RECEIVE_TIMEOUT / 2);
    Serial.println("radio sensor doesn't answer");
		return RADIO_ERROR;
	}
}

float RadioSensor::get_radio_temp() {
  return RadioSensor::radio_sens_temp;
}

void RadioSensor::clear_timeout_radio_sens(){
  RadioSensor::last_time_online = millis();
}
