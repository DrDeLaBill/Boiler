/*
 *
 * Класс, управляющий всей периферией
 * 
 */
#ifndef _BOILER_CONTROLLER_H_
#define _BOILER_CONTROLLER_H_

#include <Arduino.h>
#include "SPIFFS.h"

#include "BoilerConstants.h"
#include "BoilerProfile.h"
#include "CommandManager.h"
#include "ErrorService.h"
#include "NetworkManager.h"
#include "DisplayManager.h"
#include "EncoderManager.h"
#include "ExternalServer.h"
#include "RelayTemperature.h"
#include "PumpManager.h"
#include "PumpManager.h"
#include "TemperatureSensor.h"

class BoilerController 
{
  public:
    // режим работы: работа, режим ожидания
    static bool work_mode;
    
    BoilerController();
    static void controller_run();
};

#endif
