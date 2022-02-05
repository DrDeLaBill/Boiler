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
#include "DisplayManager.h"

class InternalServer
{
  private:
    AsyncWebServer* server;
    AsyncCallbackJsonWebHandler* handler;
  public:
    static void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
    static String get_preset(uint8_t preset_num);
    static String get_s_setpoint();
    static String get_s_profile();
    static String get_s_setpointwater();
    
    InternalServer();
    void server_init();
};

#endif
