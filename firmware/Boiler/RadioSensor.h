#ifndef _RADIO_TEMP_H
#define _RADIO_TEMP_H

#include <SPI.h> 
#include <nRF24L01.h> 
#include <RF24.h> 

#define PIN_CE 				        15 			      // Номер пина Arduino, к которому подключен вывод CE радиомодуля
#define PIN_CSN 			        2 			      // Номер пина Arduino, к которому подключен вывод CSN радиомодуля

#define RECEIVE_TIMEOUT		    200000L		    // таймаут приема данных в мс

#define GOT_EXT_TEMP 			    1
#define NO_EXT_TEMP				    0
#define RADIO_ERROR	          255			      // ошибка приема

class RadioSensor
{
  private:
    // объект radio с указанием выводов CE и CSN
    RF24 *radio; 
    // время последнего приема данных
    uint32_t last_time_online;  
  public:
    RadioSensor();
    void radio_init(void); //TODO: ext_temp_init();
    uint8_t get_radio_temp(float *temp); //TODO: get_ext_temp
    void clear_timeout_radio_sens(); //TODO: clear_timeout_ext_sens
};

#endif
