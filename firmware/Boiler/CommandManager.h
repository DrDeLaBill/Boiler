/*
 * Обработчик команд Serial порта
 */

#ifndef _COMMAND_MANAGER_H_
#define _COMMAND_MANAGER_H_

#define READ_DELAY_TIME 500 //command read delay mls

#include <Arduino.h>

#include "BoilerProfile.h"

class CommandManager
{
  private:
    String message_from_port;
    uint32_t command_read_delay;
    
    void _read_command();
    void _execute_command();
    void _clear_data();
    bool _is_time_to_read();
    String _split_string(String data, char separator, int index);
  public:
    CommandManager();
    void check_commands();
};

#endif
