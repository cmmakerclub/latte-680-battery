#ifndef CMMC_SENSOR_18B20_H
#define CMMC_SENSOR_18B20_H

#include <Arduino.h>
#include <CMMC_Sensor.h>

class CMMC_18B20 : public CMMC_Sensor
{
public: 
  OneWire *oneWire;
  DallasTemperature *sensor;
  
  CMMC_18B20() {
     this->data.type = 0x04;
  }

  ~CMMC_18B20() {
      delete oneWire;
      delete sensor; 
  }

  void setup(int pin=0, int b=0)
  {
    DeviceAddress insideThermometer;
    oneWire = new OneWire(pin);
    sensor = new DallasTemperature(oneWire); 
    sensor->begin();
    sensor->isParasitePowerMode();
    sensor->getAddress(insideThermometer, 0);
    sensor->setResolution(insideThermometer, 9);
    sensor->requestTemperatures();
    data.field1 = sensor->getTempC(insideThermometer);
  };

  void read()
  {
    static CMMC_18B20 *that = this;
    that->interval.every_ms(that->everyMs, []() {
      that->sensor->requestTemperatures();
      that->data.field1 = that->sensor->getTempCByIndex(0);
      that->cb((void *)&that->data, sizeof(that->data));
    });
  }
};

#endif