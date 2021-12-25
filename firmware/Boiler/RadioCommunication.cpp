#include "RadioCommunication.h"

RadioCommunication::RadioCommunication() {
  this->last_time_online = 0;
  this->radio_init();
}

void radio_init(){
  
	if (!radio.begin()) 
	  Serial.println("radio init error"); // Инициализация модуля NRF24L01
  else 
    Serial.println("radio init ok");
	radio.setChannel(0x6f); 
	radio.setDataRate(RF24_1MBPS); // Скорость обмена данными 1 Мбит/сек
	radio.setPALevel(RF24_PA_HIGH);           //
	radio.openReadingPipe(1, 0x7878787878LL); // Открываем трубу ID передатчика
	radio.startListening(); // Начинаем прослушивать открываемую трубу
  
  clear_timeout_ext_sens();
}

uint8_t get_ext_temp(float* pTemp){
	if (millis() - last_time_online < RECEIVE_TIMEOUT){
		if (radio.available()){
			// TODO: написать проверку приходящих данных. 
			// Возможно, добавится отправка других данных с датчика.
			
			last_time_online = millis();
			radio.read(pTemp, sizeof(float));
		  return GOT_EXT_TEMP;
		} else return NO_EXT_TEMP;
	} else {
		// time over
		last_time_online = millis() - (RECEIVE_TIMEOUT / 2);
    Serial.println("radio sensor doesn't answer");
		return RADIO_ERROR;
	}
}

void clear_timeout_ext_sens(){
  last_time_online = millis();
}
