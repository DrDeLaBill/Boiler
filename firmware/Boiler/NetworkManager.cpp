#include "NetworkManager.h"

String NetworkManager::current_ssid = "";
String NetworkManager::current_pass = "";
const String NetworkManager::soft_ap_ssid = "BoilerAP";
const String NetworkManager::soft_ap_password = "12345678";

NetworkManager::NetworkManager() {
  WiFi.mode(WIFI_MODE_STA);
  WiFi.softAP(soft_ap_ssid.c_str(), soft_ap_password.c_str());

  if(!MDNS.begin("boiler")) {
   Serial.println(F("Error starting mDNS"));
   return;
  } else {
    MDNS.addService("http", "tcp", 80);
    Serial.println(F("Network manager start"));
  }
}

void NetworkManager::connect_to_wifi(uint16_t connect_timeout){
  if (NetworkManager::current_ssid.length() != 0){
    WiFi.begin(NetworkManager::current_ssid.c_str(), NetworkManager::current_pass.c_str());

    uint32_t last_time_wifi = millis();
     
    while (WiFi.status() != WL_CONNECTED && millis() - last_time_wifi < connect_timeout){
      delay(1000);
      Serial.print(F("Connecting to WiFi.."));
      Serial.println(NetworkManager::current_ssid);
    }
    if (WiFi.status() == WL_CONNECTED){
      // когда мы подключены к WiFi сети
      Serial.print(F("ESP32 IP on the WiFi network: "));
      Serial.println(WiFi.localIP());

      NetworkManager::send_settings_to_server();
    } else {
      Serial.println(F("Couldn't connect to WiFi network."));
    }
  } else {
    Serial.println(F("WIFI client settings not found!"));
  }
}

// Совпадают ли ssid и pass BoilerProfile и NetworkManager
void NetworkManager::check_new_settings() {
  String ssid = NetworkManager::get_ssid();
  String pass = NetworkManager::get_pass();
  if (!ssid.equals(BoilerProfile::get_ssid()) || !pass.equals(BoilerProfile::get_pass())) {
    BoilerProfile::set_wifi_settings(ssid, pass);
  }
}

void NetworkManager::set_wifi_settings(String ssid, String pass) {
  Serial.println(F("Set wifi settings:"));
  Serial.print(F("ssid-"));
  Serial.println(ssid);
  Serial.print(F("pass-"));
  Serial.println(pass);
  NetworkManager::current_ssid = ssid;
  NetworkManager::current_pass = pass;
}

String NetworkManager::get_ssid() {
  return NetworkManager::current_ssid;
}

String NetworkManager::get_pass() {
  return NetworkManager::current_pass;
}

bool NetworkManager::is_wifi_connected() {
  return WiFi.status() == WL_CONNECTED;
}

uint8_t NetworkManager::get_wifi_status() {
  return WiFi.status();
}

void NetworkManager::send_settings_to_server(){}
