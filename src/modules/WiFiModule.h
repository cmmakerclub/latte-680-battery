#include <CMMC_Module.h>

#ifndef CMMC_WIFI_MODULE_H
#define CMMC_WIFI_MODULE_H 

class WiFiModule: public CMMC_Module {
  public:
    void config(CMMC_System *os, AsyncWebServer* server); 
    void setup();
    void loop(); 
  protected:
    void configWebServer();
  private:
    char sta_ssid[30] = "";
    char sta_pwd[30] = "";
    char ap_pwd[30] = ""; 
    void _init_sta(); 
};

#endif