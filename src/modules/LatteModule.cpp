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
  // bme = new CMMC_BME680();

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
  // bme->every(5000);
  // bme->setup(0, 0);
}

void LatteModule::loop() { 
  // bme->read(); 
}
