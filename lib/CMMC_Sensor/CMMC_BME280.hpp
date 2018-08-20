#ifndef CMMC_SENSOR_BME280_H
#define CMMC_SENSOR_BME280_H

#include <Arduino.h>
#include <CMMC_Sensor.h>
#include <Adafruit_BME280.h>

class CMMC_BME280 : public CMMC_Sensor
{
private:
  Adafruit_BME280 *bme; // I2C
public:

  CMMC_BME280() {
    Serial.println("Initializing Adafruit BME280.");
    bme = new Adafruit_BME280;
    this->tag = "BME280"; 
    this->data.type = 0x01;
  }

  ~CMMC_BME280() {
    delete bme; 
  }
  void setup(int a=0, int b=0 )
  {
    if (!bme->begin())
    {
      Serial.println("Could not find a valid BME280 sensor, check wiring!");
    }
  };

  void read()
  {
    static CMMC_BME280 *that = this;
    that->interval.every_ms(that->everyMs, []() {
      that->data.field1 = that->bme->readTemperature()*100;
      that->data.field2 = that->bme->readHumidity()*100;
      that->data.field3 = that->bme->readPressure()*100;
      that->cb((void *)&that->data, sizeof(that->data));
    });
  };
};

#endif