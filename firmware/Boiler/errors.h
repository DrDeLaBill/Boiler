/*
 * 
 */

#ifndef _ERRORS_H_
#define _ERRORS_H_

#include <Arduino.h>
#include "display.h"
#include "pump.h"

#define SSR_IN_PIN      36     // вход с токового трансформатора
#define CRASH_OUT_PIN   33     // выход на расцепитель


#define NOERROR         0   // нет ошибок
#define OVERHEAT        1   // перегрев (аварийный датчик), которого нет
#define PUMPBROKEN      2   // насос неисправен
#define SSRBROKEN       3   // твердотельные реле не выключаются
#define TEMPSENSBROKEN  4   // датчик температуры теплоносителя не работает
#define WATEROVERHEAT   5   // перегрев теплоносителя (обычный датчик)
#define NOPOWER         6   // нет нагрева

#define SSR_DELAY       19    // ms




void init_diagnostics();
void check_ssr_failure();
void check_failure();


#endif
