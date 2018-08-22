#ifndef CMMC_LEGEND_H
#define CMMC_LEGEND_H 
#define CMMC_USE_ALIAS 

#include <Arduino.h> 
#include "version.h"
#include <ESP8266mDNS.h>
#include <ESPAsyncTCP.h>
#include <AsyncWebSocket.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <CMMC_LED.h>
#include <CMMC_ConfigManager.h>
#include "CMMC_System.hpp"
#include <vector>
#include "CMMC_Module.h"
#include <SPIFFSEditor.h> 


static AsyncWebServer server(80);
static AsyncWebSocket ws("/ws");
static AsyncEventSource events("/events");
static CMMC_LED *blinker;

static const char* http_username = "admin";
static const char* http_password = "admin";

enum MODE {SETUP, RUN};

class CMMC_Legend: public CMMC_System {
  public:
    void addModule(CMMC_Module* module); 
    void run(); 
    void isLongPressed();
    void setup(); 

    CMMC_LED *getBlinker();
  protected: 
    void init_gpio(); 
    void init_fs();
    void init_user_sensor(); 
    void init_user_config(); 
    void init_network(); 
    bool setEnable(bool status);

  private:
    MODE mode;
    std::vector<CMMC_Module*> _modules;
    char ap_ssid[30] = "CMMC-Legend";
    void _init_ap(); 
    void setupWebServer(AsyncWebServer *server, AsyncWebSocket *ws, AsyncEventSource *events); 
    bool stopFlag = false;
};

#endif