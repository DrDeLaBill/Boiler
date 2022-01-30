#include "CommandManager.h"

CommandManager::CommandManager() {
  this->_clear_data();
  this->command_read_delay = millis();
}

void CommandManager::check_commands(){
  if (Serial.available() > 0 && this->_is_time_to_read()) {
    this->_read_command();
    this->_execute_command();
    this->command_read_delay = millis();
  }
}

void CommandManager::_read_command() {
  this->_clear_data();
  while (Serial.available() > 0) {
    this->message_from_port += Serial.readString();
  }
}

//TODO: отрефакторить boiler_id, очищать ввод
void CommandManager::_execute_command() {
  String command = this->_split_string(this->message_from_port, ' ', 0);
  command.trim();
  if (command.equals("set_boiler_id")) {
    String new_boiler_id = this->_split_string(this->message_from_port, ' ', 1);
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
  } else {
    Serial.println(F("ERROR: command syntax error"));
    Serial.print(F("No such command: '"));
    Serial.print(command);
    Serial.println(F("'"));
  }
  this->_clear_data();
}

void CommandManager::_clear_data() {
  this->message_from_port = "";
}

bool CommandManager::_is_time_to_read() {
  return abs(millis() - this->command_read_delay) > READ_DELAY_TIME;
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
