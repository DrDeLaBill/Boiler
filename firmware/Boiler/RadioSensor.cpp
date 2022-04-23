#include "RadioSensor.h"

float RadioSensor::radio_sens_temp = 0.0;
uint8_t RadioSensor::current_temperature = 0;
uint32_t RadioSensor::last_time_online = 0;

RadioSensor::RadioSensor() {
  // Инициализация модуля NRF24L01
  this->radio = new RF24(RADIO_CE_PIN, RADIO_CSN_PIN);
  if (!this->radio->begin()) {
    Serial.println("Radio module init error"); 
  }
  else {
    Serial.println("Radio module init ok");
  }
  
  this->radio->setChannel(0x6f);
  this->radio->setDataRate(RF24_1MBPS); // Скорость обмена данными 1 Мбит/сек
  this->radio->setPALevel(RF24_PA_HIGH);           //
  this->radio->openReadingPipe(1, 0x7878787878LL); // Открываем трубу ID передатчика
  this->radio->startListening(); // Начинаем прослушивать открываемую трубу
  
  RadioSensor::clear_timeout_radio_sens();
}

void RadioSensor::check_temperature(){
  RadioSensor::current_temperature = this->update_radio_temp();
}

uint8_t RadioSensor::update_radio_temp(){
	if (millis() - RadioSensor::last_time_online < RECEIVE_TIMEOUT){
		if (this->radio->available()){
			// TODO: написать проверку приходящих данных. !! валидация
			// Возможно, добавится отправка других данных с датчика.
			RadioSensor::last_time_online = millis();
			this->radio->read(&RadioSensor::radio_sens_temp, sizeof(float));
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

uint8_t get_current_temperature() {
  return RadioSensor::current_temperature;
}
