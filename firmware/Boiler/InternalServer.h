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

void start_internal_server();
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);

#endif
