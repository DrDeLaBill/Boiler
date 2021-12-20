#include "CommandManager.h"

CommandsManager::CommandsManager() {
  this->_clear_data();
}

void CommandsManager::check_commands(){
  Serial.println(F("Check serial port commands"));
  if (Serial.available() > 0) {
    this->_read_command();
    this->_execute_command();
  }
}

String get_new_boiler_id() {
  return this->new_boiler_id;
}

String get_command() {
  return this->command;
}

void CommandsManager::_read_command() {
  this->_clear_data();
  while (Serial.available() > 0) {
    this->message_from_port += Serial.read();
  }
}

void CommandsManager::_execute_command() {
  this->command = this->_split_string(this->message_from_port, ' ', 0);
  if (first_command_part == "set_boiler_id") {
    this->new_boiler_id = this->_split_string(this->message_from_port, ' ', 1);
  } else if (this->command != "get_boiler_id") {
    Serial.println(F("Command syntax error"));
  }
}

void CommandsManager::_clear_data() {
  this->command = "";
  this->message_from_port = "";
  this->new_boiler_id = "";
}

String CommandsManager::_split_string(String data, char separator, int index)
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
