#include <CMMC_Module.h>
#include <Wire.h>
#include <SSD1306.h>
#include <Adafruit_NeoPixel.h>
#include <CMMC_BME680.hpp>

#ifndef CMMC_LATTE2_MODULE_H
#define CMMC_LATTE2_MODULE_H 


extern CMMC_BME680* bme;

class LatteModule: public CMMC_Module {
  public:
    void config(CMMC_System *os, AsyncWebServer* server); 
    void setup();
    void loop(); 
    void configLoop();
  protected:
  private:
    Adafruit_NeoPixel *pixels;
    SSD1306 *oled;
};

#endif