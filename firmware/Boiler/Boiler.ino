#include "CommandManager.h"                 // -
#include "ErrorService.h"                   // const                                             | Serial, 
#include "NetworkManager.h"                 // const                                             | wifi, serial, mdns
#include "EncoderManager.h"                 // const                                             | ESP32Encoder
#include "RelayTemperature.h"               // const
#include "PumpManager.h"                    // const
#include "RadioSensor.h"                    // const                                             | RF24, serial
#include "TemperatureSensor.h"              // const                                             | DallasTemperature, GyverPID, OneWire, serial
#include "BoilerProfile.h"                  // const, TemperatureSensor, RadioSensor             | stdint, EEPROM, serial
#include "BoilerController.h"               // const, NetworkManager, BoilerProfile, PumpManager | SPIFFS, serial, ESP
#include "ExternalServer.h"                 // -                                                 | HTTPClient, ArduinoJSON
#include "InternalServer.h"                 // all                                               | AsyncJson, SPIFFS, ESPAsyncWebServer, Update
#include "DisplayManager.h"                 // const, DisplayPage, DisplayDataConfig             | U8g2lib, serial
//TODO: перевести почти всё в статик
//TODO: найти все static в .cpp

ErrorService* error_service;
NetworkManager* network_manager;
EncoderManager* encoder_manager;
RelayTemperature* relay_manager;
PumpManager* pump_manager;
RadioSensor* radio_sensor;
TemperatureSensor* temperature_sensor;
BoilerProfile* boiler_profile;
BoilerController* boiler_controller;
InternalServer* internal_server;
DisplayManager* display_manager;

void setup(){
  Serial.begin(115200);
  Serial.println(F("\n######################################################"));
  Serial.println(F("Initialization boiler started."));
  error_service = new ErrorService();
  network_manager = new NetworkManager();
  encoder_manager = new EncoderManager();
  relay_manager = new RelayTemperature();
  pump_manager = new PumpManager();
  radio_sensor = new RadioSensor();
  temperature_sensor = new TemperatureSensor();
  boiler_profile = new BoilerProfile();
  boiler_controller = new BoilerController();
  internal_server = new InternalServer();
  display_manager = new DisplayManager();
  Serial.println(F("Initialization boiler end."));
}

void loop(){
  BoilerController::controller_run();
}
