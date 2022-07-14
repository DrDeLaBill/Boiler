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

#define DEBUG true

ErrorService* error_service;
NetworkManager* network_manager;
EncoderManager* encoder_manager;
RelayTemperature* relay_manager;
PumpManager* pump_manager;
TemperatureSensor* temperature_sensor;
BoilerProfile* boiler_profile;
BoilerController* boiler_controller;
DisplayManager* display_manager;
RadioSensor* radio_sensor;

void setup(){
  #if (DEBUG)
    Serial.begin(115200);
  #endif
  Serial.println(F("\n##########################################################"));
  Serial.println(F("Initialization boiler started."));
  error_service = new ErrorService();
  encoder_manager = new EncoderManager();
  relay_manager = new RelayTemperature();
  pump_manager = new PumpManager();
  temperature_sensor = new TemperatureSensor();
  network_manager = new NetworkManager();
  boiler_profile = new BoilerProfile();
  boiler_controller = new BoilerController();
  display_manager = new DisplayManager();
  radio_sensor = new RadioSensor();
  start_internal_server();
  Serial.println(F("Initialization boiler ended."));
  Serial.println(F("______________________BOILER WORK LOG______________________"));
}


void loop(){
  BoilerController::controller_run();
}
