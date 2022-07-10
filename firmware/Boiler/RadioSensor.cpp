#include "RadioSensor.h"

float RadioSensor::radio_sens_temp = 0.0;
uint8_t RadioSensor::current_temperature = 0;
uint32_t RadioSensor::last_time_online = 0;
RF24 RadioSensor::radio(RADIO_CE_PIN, RADIO_CSN_PIN);

RadioSensor::RadioSensor() {
  // Инициализация модуля NRF24L01
  bool radio_init_state = RadioSensor::radio.begin();
  RadioSensor::radio.setChannel(0x6f);
  RadioSensor::radio.setDataRate(RF24_1MBPS); // Скорость обмена данными 1 Мбит/сек
  RadioSensor::radio.setPALevel(RF24_PA_HIGH);           //
  RadioSensor::radio.openReadingPipe(1, 0x7878787878LL); // Открываем трубу ID передатчика
  RadioSensor::radio.startListening(); // Начинаем прослушивать открываемую трубу
  this->clear_timeout_radio_sens();
  if (radio_init_state) {
    Serial.println("Radio module init ok");
  }
  else {
    Serial.println("Radio module init error");
  }
}

void RadioSensor::check_temperature(){
  RadioSensor::current_temperature = RadioSensor::update_radio_temp();
}

uint8_t RadioSensor::update_radio_temp(){
	if (millis() - RadioSensor::last_time_online < RECEIVE_TIMEOUT){
		if (RadioSensor::radio.available()){
      // TODO: написать проверку приходящих данных. !! валидация
      // Возможно, добавится отправка других данных с датчика.
      RadioSensor::last_time_online = millis();
      RadioSensor::radio.read(&RadioSensor::radio_sens_temp, sizeof(float));
      Serial.print(F("External sensor: "));
      Serial.println(RadioSensor::radio_sens_temp);
		  return GOT_EXT_TEMP;
		} else {
      ErrorService::add_error(ERROR_RADIOSENSOR);
		  return NO_EXT_TEMP;
		}
	} else {
		// time over
		RadioSensor::last_time_online = millis() - (RECEIVE_TIMEOUT / 2);
    Serial.println("radio sensor doesn't answer");
    ErrorService::add_error(ERROR_RADIOSENSOR);
		return RADIO_ERROR;
	}
}

float RadioSensor::get_radio_temp() {
  return RadioSensor::radio_sens_temp;
}

void RadioSensor::clear_timeout_radio_sens(){
  RadioSensor::last_time_online = millis();
}

uint8_t get_current_temperature() {
  return RadioSensor::current_temperature;
}
