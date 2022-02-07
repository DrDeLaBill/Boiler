#include "BoilerController.h"
//TODO: перевести почти всё в статик
//TODO: найти все static в .cpp
#define DEBUG_ON    1

InternalServer *internal_server;
BoilerController *boiler_controller;

void setup(){
  Serial.begin(115200);
  internal_server = new InternalServer();
  internal_server->internal_server_init();
  boiler_controller = new BoilerController();
}

void loop(){
  boiler_controller->controller_run();
}
