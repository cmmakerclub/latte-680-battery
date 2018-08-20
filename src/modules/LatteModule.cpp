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
  bme->onData([&](void *d, size_t len) {
    CMMC_SENSOR_DATA_T data; 
    memcpy(&data, d, len); 
    Serial.printf("DATA %lu bytes ARRIVED at %lu\r\n", len, millis()); 
    Serial.printf("field1 = %lu, field2 = %lu", data.field1, data.field2); 
    Serial.printf(" field3 = %lu, field4 = %lu", data.field3, data.field4); 
    Serial.printf(" field5 = %lu, field6 = %lu\r\n", data.field5, data.field6); 
    if (data.field4 == 0) {
      for(int i=0;i<NUMPIXELS;i++){ 
        pixels->setPixelColor(i, pixels->Color(20, 20, 20)); // calibrating
        pixels->show();
        delay(10);
      } 
      oled->clear();
      oled->drawString(20, 25, String("Calibrating.... [")+ String(millis()/1000) + String("]"));
      oled->display();
    }
    else if (data.field4 == 1) {
    oled->clear();
    oled->drawString(20, 25, String(data.field3/100));
    oled->display();
      for(int i=0;i<NUMPIXELS;i++){ 
        if (data.field3/100 <= 100) {
           pixels->setPixelColor(i, pixels->Color(0, 255 , 0)); // green
        }
        else if (data.field3/100 >= 300) {
           pixels->setPixelColor(i, pixels->Color(255, 0, 0)); // red 
        }
        else if (data.field3/100 >= 200) {
           pixels->setPixelColor(i, pixels->Color(255, 128, 0)); // orange
        }
        pixels->show();
        delay(10);
      } 
    }
    else if (data.field4 == 2) {
    oled->clear();
    oled->drawString(20, 25, String(data.field3/100));
    oled->display();
      for(int i=0;i<NUMPIXELS;i++){ 
        pixels->setPixelColor(i, pixels->Color(0, 255, 2)); // Moderately bright green color. 
        pixels->show();
        delay(10);
      } 
    }
  }); 
  bme->every(5000);
  bme->setup(0, 0);
}

void LatteModule::loop() { 
  bme->read(); 
}
