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
#include "ExternalServer.h"
#include "InternalServer.h"
//TODO: переместить relay temperaturre (проводной сенсор) в класс TEmperatureSensor
#include "RelayTemperature.h"
#include "PumpManager.h"
#include "PumpManager.h"

class BoilerController 
{
  private:
    // режим работы: работа, режим ожидания
    bool work_mode;
    ErrorService *error_service;
    NetworkManager *network_manager;
    ExternalServer *external_server;
    InternalServer *internal_server;
    BoilerProfile *boiler_profile;
    CommandManager *command_manager;
    EncoderManager *encoder_manager;
    RelayTemperature *relay_manager;
    PumpManager *pump_manager;
    DisplayManager *display_manager;

    void _check_serial_port_commands();
    void _check_network_settings();
    void _fill_display_manager_configuration();
    void _check_button_commands();
    void _check_external_server_sttings();
    void _external_profile_settings_init(String url);
  public:
    BoilerController();
    void controller_init();
    void controller_run();
    void serial_error_report(String target_url, int responce_code);
};

#endif
