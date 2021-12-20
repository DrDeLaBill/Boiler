//TODO: extern and include
#include "network.h"
extern const char* S_SETPOINT;
extern const char* S_SETPOINTWATER;
extern const char* S_PROFILE;
extern const char* S_WEEKDAYS;
extern const char* S_WEEKEND;
extern const char* S_CUSTOM;
extern const char* S_NOTFREEZE;

extern const char* presets[];
extern BoilerConfig BoilerCfg;
extern uint8_t current_temp;
extern uint8_t user_boiler_mode;

class ExternalServer
{
  private:
    HTTPClient http;
    const char* HEADER_TYPE = "Content-Type";
    const char* JSON_HEADER = "application/json";
    const char* WebServerAddr = "http://192.168.10.199";
    const char* boiler_id = "abcdabcd12";
    bool connected_to_server;
    bool got_new_wifi_settings;
  public:
    ExternalServer();
    send_settings_to_server(void);
    check_new_settings(void);
};
