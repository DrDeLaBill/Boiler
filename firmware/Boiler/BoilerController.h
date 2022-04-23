/*
 *
 * Класс, управляющий всей периферией
 * 
 */
#ifndef _BOILER_CONTROLLER_H_
#define _BOILER_CONTROLLER_H_

#include <Arduino.h>
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
#include "RadioSensor.h"
#include "TemperatureSensor.h"

class BoilerController 
{
  public:
    // режим работы: работа, режим ожидания
    static bool work_mode;
    
    BoilerController();
    static void controller_run();
    static ErrorService error_service;
    static NetworkManager network_manager;
    // Беспроводной сенсор температуры воздуха
    static RadioSensor radio_sensor;
    static TemperatureSensor temperature_sensor;
    static CommandManager command_manager;
    static BoilerProfile boiler_profile;
    static DisplayManager display_manager;
    static EncoderManager encoder_manager;
    static RelayTemperature relay_manager;
    static PumpManager pump_manager;
};

#endif
