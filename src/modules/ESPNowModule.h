
#ifndef CMMC_ESPNOW_MODULE_H
#define CMMC_ESPNOW_MODULE_H
#define CMMC_USE_ALIAS 

#include <CMMC_Legend.h>
#include <CMMC_Module.h>
#include <CMMC_ESPNow.h>
#include <CMMC_SimplePair.h>
#include <CMMC_LED.h>
#include <CMMC_Interval.h>
#include <CMMC_BME680.hpp>

#define BUTTON_PIN  0

extern CMMC_BME680* bme;

class ESPNowModule: public CMMC_Module {
  public:
    void config(CMMC_System *os, AsyncWebServer* server); 
    void configLoop(); 
    void setup(); 
    void loop(); 
  private:
    char _sensorName[16];

    uint8_t _defaultDeepSleep_m = 30;
    CMMC_Interval sendingInterval; 
    CMMC_System *os; 
    CMMC_ESPNow espNow;
    CMMC_SimplePair simplePair;
    CMMC_LED *led;
    uint8_t self_mac[6];
    uint8_t master_mac[6]; 
    bool sp_flag_done = false; 
    void _init_simple_pair(); 
    void _go_sleep(uint32_t deepSleepM); 
    void _init_espnow();
    typedef struct __attribute((__packed__)) {
      uint8_t from[6];
      uint8_t to[6];
      uint8_t type = 0;
      uint32_t battery = 0x00;
      int32_t field1 = 1;
      int32_t field2 = 2;
      int32_t field3 = 3;
      int32_t field4 = 4;
      int32_t field5 = 5;
      int32_t field6 = 6;
      int32_t field7 = 7;
      int32_t field8 = 8;
      int32_t field9 = 9;
      uint8_t nameLen = 15;
      char sensorName[16];
      uint32_t ms = 0;
      uint32_t sent_ms = 0;
      uint32_t sum = 0;
    } CMMC_SENSOR_DATA_T;
  CMMC_SENSOR_DATA_T packet; 
}; 


#endif