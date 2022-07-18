#ifndef _SSR_TEMP_H
#define _SSR_TEMP_H

#include <Arduino.h>

#include "BoilerConstants.h"
#include "ErrorService.h"
#include "DisplayManager.h"

#define SSR_TEMP_PIN		        39

#define SSR_AN_VAL_0            3580        // значение ацп при 0 градусов
#define SSR_TEMP_KOEF           42          // количество значений ацп на градус (примерно)

#define SSR_TEMP_BUF_SIZE		    10		      // количество сэмплов для усреднения

#define SSR_TEMP_UPPER_LIM      60          // *C
#define SSR_TEMP_LOWER_LIM      50          // *C

#define SSR_BROKEN_TIMEOUT      180000      // в течении этого времени ТТР должны остыть, мс

class RelayTemperature
{
  public:
    static uint16_t ssr_temp_buf[SSR_TEMP_BUF_SIZE];
    static uint32_t ssr_temp_summ;
    static uint32_t ssr_broken_last_time;
    
    RelayTemperature();
    static void check_ssr_temp();
    static bool is_heating_on();
};

#endif
