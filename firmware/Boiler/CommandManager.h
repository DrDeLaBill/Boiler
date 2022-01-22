/*
 * Обработчик команд Serial порта
 */

#ifndef _COMMAND_MANAGER_H_
#define _COMMAND_MANAGER_H_

#include <Arduino.h>

class CommandManager
{
  private:
    String message_from_port;
    String command;
    String new_boiler_id;
    
    void _read_command();
    void _execute_command();
    void _clear_data();
    String _split_string(String data, char separator, int index);
  public:
    CommandManager();
    void check_commands();
    String get_new_boiler_id();
    String get_command();
    void set_boiler_id(String new_boiler_id);
};

#endif
