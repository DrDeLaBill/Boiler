/*
 * Обработчик команд Serial порта
 */

#ifndef _COMMAND_MANAGER_H_
#define _COMMAND_MANAGER_H_

#include <Arduino.h>
#include "EEPROM.h"

#include "BoilerProfile.h"

class CommandManager
{
  public:
    static String message_from_port;
    static void check_commands();
    static void _read_command();
    static void _execute_command();
    static void _clear_data();
    static String _split_string(String data, char separator, int index);
};

#endif
