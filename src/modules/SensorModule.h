#include <CMMC_Module.h>
#include <CMMC_Sensor.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <DHT.h>
#include <Wire.h>
#include <SPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#ifndef CMMC_SENSOR_MODULE_H
#define CMMC_SENSOR_MODULE_H 

class SensorModule: public CMMC_Module {
  public:
    void config(CMMC_System *os, AsyncWebServer* server); 
    void setup();
    void loop(); 
  protected:
    void configWebServer();
  private:
    char sensorName[20];
    CMMC_Sensor *sensor;
};

#endif