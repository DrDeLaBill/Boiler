/*
 * 
 */

#ifndef _PUMP_H_INCLUDED_
#define _PUMP_H_INCLUDED_

#include <Arduino.h>


#define PUMP_OUT_PIN          32        // пин на насос
#define FLOW_IN_PIN           34        // вход с датчика протока

#define PUMP_OFF              0
#define PUMP_ON               1
#define PUMP_WAIT             2

#define PUMP_BROKEN           1         // насос неисправен

#define PUMP_OFF_TIMEOUT       60000    // длительность работы насоса после выключения котла

class PumpManager
{
  private:
    uint8_t pump_state;
    uint32_t pump_off_delay;
  public:
    PumpManager();
    void pump_init();
    void pump_on();
    void pump_off();
    uint8_t check_pump();
};

#endif
