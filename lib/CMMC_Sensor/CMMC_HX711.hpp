#ifndef CMMC_SENSOR_HX711_H
#define CMMC_SENSOR_HX711_H

#include <Arduino.h>
#include <CMMC_Sensor.h>
#include "HX711.h"

// real weight 670 g
#define zero_factor 8616207

class CMMC_HX711 : public CMMC_Sensor
{
public:
  float calibration_factor = 9051.00;
  float offset = 0.0;
  String dataKG;

  HX711 *scale;

  float get_units_kg()
  {
    return (scale->get_units() * 0.453592);
  }

  void setup()
  {
    setup(0, 0);
  }

  void setup(int DOUT = 0, int CLK = 0)
  {
    Serial.println("HX711 initialized.");
    // DOUT 12  ||   CLK 14
    scale = new HX711(DOUT, CLK);
    scale->set_scale(calibration_factor);
    scale->set_offset(zero_factor);

    dataKG = String((get_units_kg() * 100) + (offset * 100), 2);

    Serial.print("cal scale = ");
    Serial.println(dataKG.toInt());
    data.field1 = dataKG.toInt();
  };

  void read()
  {
    data.ms = millis();
    dataKG = String((get_units_kg() * 100) + (offset * 100), 2);
    data.field1 = dataKG.toInt();
    
    static CMMC_HX711 *that = this;
    that->interval.every_ms(that->everyMs, []() {
      that->cb((void *)&that->data, sizeof(that->data));
    });
  };
};

#endif