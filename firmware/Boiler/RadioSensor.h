#ifndef _RADIO_TEMP_H
#define _RADIO_TEMP_H

#include <SPI.h> 
#include <nRF24L01.h> 
#include <RF24.h>
#include "ErrorService.h"

#define RADIO_CE_PIN 	        15 			      // Номер пина Arduino, к которому подключен вывод CE радиомодуля
#define RADIO_CSN_PIN         2 			      // Номер пина Arduino, к которому подключен вывод CSN радиомодуля

#define RECEIVE_TIMEOUT		    120000		    // таймаут приема данных в мс

#define NO_EXT_TEMP				    0
#define GOT_EXT_TEMP          1
#define RADIO_ERROR	          255			      // ошибка приема

class RadioSensor
{
  public:
    // объект radio с указанием выводов CE и CSN
    static RF24 radio;
    // время последнего приема данных
    static uint32_t last_time_online;
    static uint8_t current_sensor_state;
    static uint8_t sensor_state;
    static float radio_sens_temp;
    
    RadioSensor();
    static uint8_t update_radio_temp();
    static void check_temperature();
    static float get_radio_temp();
    static void clear_timeout_radio_sens();
    static uint8_t get_current_sensor_state();
    static bool is_sensor_online();
};

#endif
