#include "CommandManager.h"

String CommandManager::message_from_port = "";

void CommandManager::check_commands(){
  if (Serial.available() > 0) {
    CommandManager::_read_command();
    CommandManager::_execute_command();
  }
}

void CommandManager::_read_command() {
  CommandManager::_clear_data();
  while (Serial.available() > 0) {
    CommandManager::message_from_port += Serial.readString();
  }
}

void CommandManager::_execute_command() {
  String command = CommandManager::_split_string(CommandManager::message_from_port, ' ', 0);
  command.trim();
  if (command.equals("set_boiler_id")) {
    String new_boiler_id = CommandManager::_split_string(CommandManager::message_from_port, ' ', 1);
    Serial.println(F("Setting new boiler ID"));
    Serial.print(F("Old boiler ID: "));
    Serial.println(BoilerProfile::get_boiler_id());
    BoilerProfile::set_boiler_id(new_boiler_id);
    Serial.println(F("Boiler ID from command: "));
    Serial.print(new_boiler_id);
    Serial.println(F("New Boiler ID: "));
    Serial.print(BoilerProfile::get_boiler_id());
    Serial.println();
  } else if (command.equals("get_boiler_id")) {
    String boiler_id = BoilerProfile::get_boiler_id();
    Serial.print(F("Get boiler ID: "));
    Serial.println(boiler_id);
  } else if (command.equals("set_default_settings")){
    BoilerProfile::set_default_settings();
    ErrorService::clear_errors();
  } else {
    Serial.println(F("ERROR: command syntax error"));
    Serial.print(F("No such command: '"));
    Serial.print(command);
    Serial.println(F("'"));
  }
  CommandManager::_clear_data();
}

void CommandManager::_clear_data() {
  CommandManager::message_from_port = "";
}

String CommandManager::_split_string(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}
