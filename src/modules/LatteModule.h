#include <CMMC_Module.h>
#include <Wire.h>
#include <SSD1306.h>
#include <Adafruit_NeoPixel.h>
#include <CMMC_BME680.hpp>
#include <VL53L0X.h>


#ifndef CMMC_LATTE2_MODULE_H
#define CMMC_LATTE2_MODULE_H 


extern CMMC_BME680* bme;

class LatteModule: public CMMC_Module {
  public:
    void config(CMMC_System *os, AsyncWebServer* server); 
    void setup();
    void loop(); 
    void configLoop();
    uint16_t getDistanceMillimeters();
  protected:
  private:
    Adafruit_NeoPixel *pixels;
    SSD1306 *oled;
    VL53L0X *vl53l0x;
};

#endif