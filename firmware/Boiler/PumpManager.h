#ifndef _PUMP_H_INCLUDED_
#define _PUMP_H_INCLUDED_

#include <Arduino.h>

#include "BoilerConstants.h"

#define PUMP_OFF              0
#define PUMP_ON               1
#define PUMP_WAIT             2

#define PUMP_BROKEN           1         // насос неисправен

#define PUMP_OFF_TIMEOUT      60000    // длительность работы насоса после выключения котла

class PumpManager
{
  public:
    static uint8_t pump_state;
    static uint32_t pump_off_delay;
    
    PumpManager();
    static void pump_init();
    static void pump_on();
    static void pump_off();
    static uint8_t check_pump();
};

#endif
