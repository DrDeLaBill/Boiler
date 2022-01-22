#ifndef _INTERNAL_SERVER_H_
#define _INTERNAL_SERVER_H_

#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <Update.h>
#include "SPIFFS.h"

#include "BoilerConstants.h"
#include "BoilerProfile.h"
#include "NetworkManager.h"
#include "ExternalServer.h"
#include "TemperatureSensor.h"
#include "ClockRTC.h"

class InternalServer
{
  private:
    AsyncWebServer* server;
    AsyncCallbackJsonWebHandler* handler;
  public:
    static void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
    
    InternalServer();
    void server_init();
    String get_preset(uint8_t preset_num);
    String get_s_setpoint();
    String get_s_profile();
    String get_s_setpointwater();
};

#endif
