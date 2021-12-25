/*
 * 
 * Класс, управляющий всей периферией
 * 
 */
#ifndef _BOILER_CONTROLLER_H_
#define _BOILER_CONTROLLER_H_

// Режимы работы
#define MODE_STANDBY          true
#define MODE_WORK             false

#include <Arduino.h>
#include "BoilerProfile.h"
#include "CommandManager.h"
#include "ErrorService.h"
#include "NetworkManager.h"
#include "DisplayManager.h"
#include "EncoderManager.h"

class BoilerController 
{
  private:
    bool work_mode;
    bool p_mode;         // режим работы: работа, режим ожидания
    NetworkManager network_manager;
    ExternalServer external_server;
    InternalServer internal_server;
    BoilerProfile boiler_profile;
    CommandManager command_manager;
    ErrorService error_servise;
    EncoderManager encoder_manager;
    RelayTemperature relay_manager;
    PumpManager pump_manager;
    DisplayManager display_manager;

    void _check_serial_port_commands();
    void _check_network_settings();
    void _fill_display_manager_configuration();
  public:
    BoilerController();
    void controller_init();
    void controller_run();
};

#endif
