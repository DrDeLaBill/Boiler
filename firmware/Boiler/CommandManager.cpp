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
    Serial.print(F("Boiler ID from command: "));
    Serial.println(new_boiler_id);
    BoilerProfile::save_configuration();
  } else if (command.equals("set_ssid")) {
    String ssid = CommandManager::_split_string(CommandManager::message_from_port, ' ', 1);
    Serial.println(F("Setting SSID"));
    Serial.print(F("Old SSID: "));
    Serial.println(BoilerProfile::boiler_configuration.ssid);
    ssid.trim();
    ssid.toCharArray(BoilerProfile::boiler_configuration.ssid, MAX_SIZE_SSID);
    NetworkManager::current_ssid = ssid;
    ExternalServer::got_new_wifi_settings = true;
    Serial.print(F("New SSID: "));
    Serial.println(ssid);
    BoilerProfile::save_configuration();
  } else if (command.equals("set_ssid_pass")) {
    String ssid_pass = CommandManager::_split_string(CommandManager::message_from_port, ' ', 1);
    Serial.println(F("Setting SSID PASS"));
    Serial.print(F("Old SSID PASS: "));
    Serial.println(BoilerProfile::boiler_configuration.password);
    ssid_pass.trim();
    ssid_pass.toCharArray(BoilerProfile::boiler_configuration.password, MAX_SIZE_PASS);
    NetworkManager::current_pass = ssid_pass;
    ExternalServer::got_new_wifi_settings = true;
    Serial.print(F("Boiler SSID PASS: "));
    Serial.println(ssid_pass);
    BoilerProfile::save_configuration();
  } else if (command.equals("get_boiler_id")) {
    String boiler_id = BoilerProfile::get_boiler_id();
    Serial.print(F("Get boiler ID: "));
    Serial.println(boiler_id);
  } else if (command.equals("set_default_settings")){
    Serial.println(F("SET DEFAULT SETTINGS"));
    BoilerProfile::set_default_settings();
    ErrorService::clear_errors();
  } else if (command.equals("clear_eeprom")){
    Serial.println(F("CLEAR EEPROM"));
    BoilerProfile::clear_eeprom();
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
