#ifndef EXT_TEMP_H
#define EXT_TEMP_H

#include <SPI.h> 
#include <nRF24L01.h> 
#include <RF24.h> 

#define PIN_CE 				        15 			    // Номер пина Arduino, к которому подключен вывод CE радиомодуля
#define PIN_CSN 			        2 			    // Номер пина Arduino, к которому подключен вывод CSN радиомодуля

#define RECEIVE_TIMEOUT		    200000L		  // таймаут приема данных в мс

#define GOT_EXT_TEMP 			    1
#define NO_EXT_TEMP				    0
#define RADIO_ERROR	          255			    // ошибка приема

class RadioCommunication
{
  private:
    // объект radio с указанием выводов CE и CSN
    RF24 radio(PIN_CE, PIN_CSN); 
    // время последнего приема данных
    uint32_t last_time_online;  
  public:
    void radio_init(void);
    uint8_t get_ext_temp(float *temp);
    void clear_timeout_ext_sens();
};

#endif
