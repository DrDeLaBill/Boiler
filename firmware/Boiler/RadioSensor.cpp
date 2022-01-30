#include "RadioSensor.h"

RadioSensor::RadioSensor() {
  this->radio = new RF24(PIN_CE, PIN_CSN); 
  this->last_time_online = 0;
  this->radio_init();
}

void RadioSensor::radio_init(){
  // Инициализация модуля NRF24L01
	if (!this->radio->begin()) {
	  Serial.println("radio init error"); 
	}
  else {
    Serial.println("radio init ok");
  }
  
	this->radio->setChannel(0x6f);
	this->radio->setDataRate(RF24_1MBPS); // Скорость обмена данными 1 Мбит/сек
	this->radio->setPALevel(RF24_PA_HIGH);           //
	this->radio->openReadingPipe(1, 0x7878787878LL); // Открываем трубу ID передатчика
	this->radio->startListening(); // Начинаем прослушивать открываемую трубу
  
  this->clear_timeout_radio_sens();
}

uint8_t RadioSensor::get_radio_temp(float* pTemp){
	if (millis() - this->last_time_online < RECEIVE_TIMEOUT){
		if (this->radio->available()){
			// TODO: написать проверку приходящих данных. 
			// Возможно, добавится отправка других данных с датчика.
			
			this->last_time_online = millis();
			this->radio->read(pTemp, sizeof(float));
		  return GOT_EXT_TEMP;
		} else return NO_EXT_TEMP;
	} else {
		// time over
		this->last_time_online = millis() - (RECEIVE_TIMEOUT / 2);
    Serial.println("radio sensor doesn't answer");
		return RADIO_ERROR;
	}
}

void RadioSensor::clear_timeout_radio_sens(){
  this->last_time_online = millis();
}
