#include "NetworkManager.h"

NetworkManager::NetworkManager() {
    this->current_ssid = "";
    this->current_pass = "";
    this->network_init();
}

void NetworkManager::network_init(){
  WiFi.mode(WIFI_MODE_STA);
  WiFi.softAP(soft_ap_ssid, soft_ap_password);
  Serial.println(F("WIFI AP has been set"));
  Serial.print(F("ESP32 IP as soft AP: "));
  Serial.println(WiFi.softAPIP());

  this->connect_to_wifi();

  if(!MDNS.begin("boiler")) {
   Serial.println(F("Error starting mDNS"));
   return;
  }

  MDNS.addService("http", "tcp", 80);

  this->server_init();
}

void NetworkManager::connect_to_wifi(void){
  if (this->current_ssid.length() != 0){
    WiFi.begin(this->current_ssid.c_str(), this->current_password.c_str());

    uint32_t last_time_wifi = millis();
     
    while (WiFi.status() != WL_CONNECTED && millis() - last_time_wifi < WIFI_CONNECT_TIMEOUT){
      delay(1000);
      Serial.print("Connecting to WiFi..");
      Serial.println(this->current_ssid);
    }
    if (WiFi.status() == WL_CONNECTED){
      // когда мы подключены к WiFi сети
      Serial.print(F("ESP32 IP on the WiFi network: "));
      Serial.println(WiFi.localIP());

      this->send_settings_to_server();
    } else {
      Serial.println(F("Couldn't connect to WiFi network."));
    }
  } else {
    Serial.println(F("WIFI client settings not found!"));
  }
}

void NetworkManager::set_wifi_settings(String ssid, String pass) {
  this->current_ssid = ssid;
  this->current_pass = pass;
}

String NetworkManager::get_ssid() {
  return this->current_ssid;
}

String NetworkManager::get_pass() {
  return this->current_pass;
}

bool is_wifi_connected() {
  return WiFi.status() == WL_CONNECTED;
}

void NetworkManager::server_init(){}
void NetworkManager::send_settings_to_server(void){}
void NetworkManager::check_new_settings(void){}
