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

class BoilerController 
{
  private:
    bool work_mode;
    BoilerProfile boiler_profile;
    CommandManager command_manager;
    ErrorService error_servise;
    NetworkManager network_manager;
    DisplayManager display_manager;

    void _check_serial_port_commands();
    void _check_network_settings();
  public:
    void controller_init();
    void controller_run();
};

#endif
