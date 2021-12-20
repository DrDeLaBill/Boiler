#include "network.h"

// текущие настройки ssid & pass для подключения к роутеру
String current_ssid = "";
String current_password = "";

// ssid и pass точки доступа котла
const char *soft_ap_ssid = "BoilerAP";        
const char *soft_ap_password = "12345678";

void connect_to_wifi(void){
  if (current_ssid.length() != 0){
    WiFi.begin(current_ssid.c_str(), current_password.c_str());

    uint32_t last_time_wifi = millis();
     
    while (WiFi.status() != WL_CONNECTED && millis() - last_time_wifi < WIFI_CONNECT_TIMEOUT){
      delay(1000);
      Serial.print("Connecting to WiFi..");
      Serial.println(current_ssid);
    }
    if (WiFi.status() == WL_CONNECTED){
      // когда мы подключены к WiFi сети
      Serial.print("ESP32 IP on the WiFi network: ");
      Serial.println(WiFi.localIP());

      send_settings_to_server();
    } else {
      Serial.println("Couldn't connect to WiFi network.");
    }
  } else {
    Serial.println("WIFI client settings not found!");
  }
}

void network_init(){
  WiFi.mode(WIFI_MODE_STA);
  WiFi.softAP(soft_ap_ssid, soft_ap_password);
  Serial.println("WIFI AP has been set");
  Serial.print("ESP32 IP as soft AP: ");
  Serial.println(WiFi.softAPIP());

  connect_to_wifi();

  if(!MDNS.begin("boiler")) {
   Serial.println("Error starting mDNS");
   return;
  }

  MDNS.addService("http", "tcp", 80);

  server_init();
}
