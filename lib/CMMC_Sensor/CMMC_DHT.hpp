#ifndef CMMC_SENSOR_DHT_H
#define CMMC_SENSOR_DHT_H

#include <Arduino.h>
#include <CMMC_Sensor.h>

class CMMC_DHT : public CMMC_Sensor
{
public:
  DHT *dht;
  int pin;
  int type;

  CMMC_DHT(int type = 0, int pin = 0) {
    this->data.type = 0x03;
    this->pin = pin;
    this->type = type;
    dht = new DHT(pin, type); 
  }

  ~CMMC_DHT() {
    delete dht; 
  }

  void setup(int pin, int type)
  {
    Serial.printf("DHT BEGIN... pin=%d, type=%d\r\n", pin, type);
    this->tag = String("DHT")+type;
    dht->begin();
  };

  void read()
  {
    static CMMC_DHT *that = this;
    that->interval.every_ms(that->everyMs, []() {
      float t,h;
      t = that->dht->readTemperature()*100;
      h = that->dht->readHumidity()*100;
      if (isnan(t) || isnan(h)) {
        // that->data.field1 = (0); 
        // that->data.field2 = (0); 
      }
      else {
        Serial.printf(">> temp=%f, humid=%f\r\n", t,h);
        that->data.field1 = (t);
        that->data.field2 = (h); 
      }
      that->cb((void *)&that->data, sizeof(that->data));
    });
  }
}; 
#endif