#include "WiFiModule.h"

#define WIFI_CONFIG_FILE "/wifi.json"

void WiFiModule::config(CMMC_System *os, AsyncWebServer* server) {
  strcpy(this->path, "/api/wifi/sta");
  static WiFiModule *that = this;
  this->_serverPtr = server;
  this->_managerPtr = new CMMC_ConfigManager(WIFI_CONFIG_FILE);
  this->_managerPtr->init();
  this->_managerPtr->load_config([](JsonObject * root, const char* content) {
    if (root == NULL) {
      Serial.print("wifi.json failed. >");
      Serial.println(content);
      return ;
    }
    Serial.println("[user] wifi config json loaded..");
    const char* sta_config[2];
    sta_config[0] = (*root)["sta_ssid"];
    sta_config[1] = (*root)["sta_password"];
    if ((sta_config[0] == NULL) || (sta_config[1] == NULL)) {
      Serial.println("NULL..");
      SPIFFS.remove("/enabled");
      return;
    };
    strcpy(that->sta_ssid, sta_config[0]);
    strcpy(that->sta_pwd, sta_config[1]);
  });
  this->configWebServer();
}

void WiFiModule::configWebServer() {
  static WiFiModule *that = this;
  _serverPtr->on(this->path, HTTP_POST, [&](AsyncWebServerRequest * request) {
    String output = that->saveConfig(request, this->_managerPtr);
    request->send(200, "application/json", output);
  }); 
}
void WiFiModule::setup() {
  _init_sta();
}

void WiFiModule::loop() { }
void WiFiModule::_init_sta() {
  WiFi.softAPdisconnect();
  WiFi.disconnect();
  delay(20);
  WiFi.mode(WIFI_STA);
  delay(20);
  // WiFi.hostname(ap_ssid);
  delay(20);
  WiFi.begin(sta_ssid, sta_pwd);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf ("Connecting to %s:%s\r\n", sta_ssid, sta_pwd);
    // os->isLongPressed();
    delay(300);
  }
  Serial.println("WiFi Connected."); 
}

