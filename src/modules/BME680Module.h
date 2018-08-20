#include <CMMC_Module.h>
#include <CMMC_BME680.hpp>

#ifndef CMMC_BME680_MODULE_H
#define CMMC_BME680_MODULE_H 

class BME680Module: public CMMC_Module {
  public:
    void config(CMMC_System *os, AsyncWebServer* server); 
    void setup();
    void loop(); 
    void configLoop();
  protected:
  private:
    CMMC_BME680 *bme;
};

#endif