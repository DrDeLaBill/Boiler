//TODO: include
//#include "SPIFFS.h"
#include "OneParamRewrite.h"

//TODO: include and extern
#include "network.h"
extern String current_ssid;
extern String current_password;
extern bool got_new_wifi_settings;
extern bool connected_to_server;

extern BoilerConfig BoilerCfg;
extern uint8_t user_boiler_mode;
extern uint8_t current_temp;
extern uint8_t radio_connected;

class InternalServer
{
  private:
    AsyncWebServer server(80);
    AsyncCallbackJsonWebHandler* handler;
    
    // параметры переменных для REST API
    const char* S_SSID = "ssid";
    const char* S_PASSWORD = "password";
    const char* S_DT = "dt";
    const char* S_TZ = "tz";
    const char* S_NAME = "name";
    const char* S_WEEKDAYS = "weekdays";
    const char* S_WEEKEND = "weekend";
    const char* S_CUSTOM = "custom";
    const char* S_NOTFREEZE = "notfreeze";
    const char* S_H0 = "h0";
    const char* S_H1 = "h1";
    const char* S_H2 = "h2";
    const char* S_H3 = "h3";
    const char* S_H4 = "h4";
    const char* S_H5 = "h5";
    const char* S_SETPOINT  = "setpoint";
    const char* S_SETPOINTWATER = "setpointwater";
    const char* S_PROFILE = "profile";
    
    const char* presets[NUM_PRESETS] = {S_WEEKDAYS, S_WEEKEND, S_CUSTOM, S_NOTFREEZE};
    
    const char* http_login = "boilerClient";    // логин и пароль для входа на сайт
    const char* http_pass = "df78jhl<z";
  public:
    InternalServer();
    void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
    void server_init(void);
};
