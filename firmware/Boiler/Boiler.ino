#include "BoilerController.h"

#define DEBUG_ON    1

BoilerController *boiler_controller;

void setup(){
  boiler_controller = new BoilerController();
}

void loop(){
  boiler_controller->controller_run();
}
