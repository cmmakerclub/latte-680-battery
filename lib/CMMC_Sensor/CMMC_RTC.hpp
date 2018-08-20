#ifndef CMMC_SENSOR_RTC_H
#define CMMC_SENSOR_RTC_H

#include <Arduino.h>
#include <CMMC_Sensor.h>
#include <Wire.h>
#include "RTClib.h"

class CMMC_RTC : public CMMC_Sensor
{
public:
  typedef struct SENSOR_DATA
  {
    uint32_t hour;
    uint32_t minute;
    uint32_t second;

    uint32_t day;
    uint32_t month;
    uint32_t year;

    uint32_t daysOfWeek;
  };

  SENSOR_DATA data;
  RTC_DS1307 *rtc;
  

  void setup(int a = 0, int b = 0)
  {
    rtc = new RTC_DS1307;
  

    rtc->begin();
    if (!rtc->isrunning()) {
      rtc->adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    DateTime now = rtc->now();

    data.hour = now.hour();
    data.minute = now.hour();
    data.second = now.second();
    data.day = now.day();
    data.month = now.month();
    data.year = now.year();
  };

  void read()
  {
    static CMMC_RTC *that = this;
    that->interval.every_ms(that->everyMs, []() {
      // that->sensor->requestTemperatures();
      // that->data.temperature = that->sensor->getTempCByIndex(0);
      that->cb((void *)&that->data, sizeof(that->data));
    });
  }
};

#endif