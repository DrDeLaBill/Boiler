#include "BoilerController.h"
#include "InternalServer.h"
//TODO: перевести почти всё в статик
//TODO: найти все static в .cpp

InternalServer internal_server();
BoilerController boiler_controller();

void setup(){
  Serial.begin(115200);
  InternalServer::internal_server_init();
}

void loop(){
  BoilerController::controller_run();
}
