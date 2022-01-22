#ifndef _INTERNAL_SERVER_H_
#define _INTERNAL_SERVER_H_

#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <Update.h>
#include "SPIFFS.h"

#include "OneParamRewrite.h"
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
    InternalServer();
    static void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
    void server_init();
    char* get_preset(uint8_t preset_num);
    char* get_s_setpoint();
    char* get_s_profile();
    char* get_s_setpointwater();
};

#endif
