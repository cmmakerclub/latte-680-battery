#ifndef CMMC_SENSOR_VL53L0X_H
#define CMMC_SENSOR_VL53L0X_H

#include <Arduino.h>
#include <CMMC_Sensor.h>
#include "Adafruit_VL53L0X.h"


class CMMC_VL53L0X: public CMMC_Sensor
{
public:
  // Adafruit_VL53L0X lox = Adafruit_VL53L0X();
  Adafruit_VL53L0X *lox;

  void setup()
  {
    setup(0, 0);
  }

  void setup(int DOUT = 0, int CLK = 0)
  {
    Serial.println("VL53L0X initialized.");
    
    lox = new Adafruit_VL53L0X(Adafruit_VL53L0X());
  
    lox->begin();

    VL53L0X_RangingMeasurementData_t measure;
    lox->rangingTest(&measure, false);
    Serial.print("Distance (mm): ");
    Serial.println(measure.RangeMilliMeter);
    data.field1 = measure.RangeMilliMeter;
  };

  void read()
  {
    VL53L0X_RangingMeasurementData_t measure;
    lox->rangingTest(&measure, false);
    data.field1 = measure.RangeMilliMeter;
    data.ms = millis();
    
    static CMMC_VL53L0X *that = this;
    that->interval.every_ms(that->everyMs, []() {
      that->cb((void *)&that->data, sizeof(that->data));
    });
  };
};

#endif