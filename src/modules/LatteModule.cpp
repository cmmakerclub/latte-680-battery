#include "LatteModule.h"

#define NUMPIXELS      7


void LatteModule::config(CMMC_System *os, AsyncWebServer* server) {
  static LatteModule *that = this;
  pixels  = new Adafruit_NeoPixel(NUMPIXELS, 15, NEO_GRB + NEO_KHZ800);
  oled  = new SSD1306(0x3c, 4, 5);
  pixels->begin();
}

void LatteModule::configLoop() { 
   for(int i=0;i<NUMPIXELS;i++){ 
    pixels->setPixelColor(i, pixels->Color(253, 10 , 2)); // Moderately bright green color. 
    pixels->show();
    delay(10);
  } 
}

void LatteModule::setup() {
  bme = new CMMC_BME680(); 
  vl53l0x = new VL53L0X;
  bme->setup();
  bme->read();

  vl53l0x->init();
  vl53l0x->setTimeout(500);
  vl53l0x->setSignalRateLimit(0.1);

  vl53l0x->setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
  vl53l0x->setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);

  oled  = new SSD1306(0x3c, 4, 5);
  pixels  = new Adafruit_NeoPixel(NUMPIXELS, 15, NEO_GRB + NEO_KHZ800); 
  pixels->begin();
  
  oled->init();
  oled->flipScreenVertically();
  oled->setContrast(255);
  oled->clear();
  oled->setFont(ArialMT_Plain_10); 
  oled->println("starting...");
  oled->display();
}

uint16_t LatteModule::getDistanceMillimeters() {
  uint16_t distanceMillimeters = vl53l0x->readRangeSingleMillimeters();
  Serial.println(distanceMillimeters);
  return distanceMillimeters;
}

void LatteModule::loop() { 
  bme->read(); 
}
