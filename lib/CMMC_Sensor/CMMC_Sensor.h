#ifndef CMMC_SENSOR_H
#define CMMC_SENSOR_H
#include <CMMC_Interval.h>
  typedef struct __attribute((__packed__)) {
    uint8_t from[6];
    uint8_t to[6];
    uint8_t type = 0;
    uint32_t battery = 0x00;
    uint32_t field1 = 1;
    uint32_t field2 = 2;
    uint32_t field3 = 3;
    uint32_t field4 = 4;
    uint32_t field5 = 5;
    uint32_t field6 = 6;
    uint32_t field7 = 7;
    uint32_t field8 = 8;
    uint32_t field9 = 9;
    uint8_t nameLen = 15;
    char sensorName[16];
    uint32_t ms = 0;
    uint32_t sent_ms = 0;
    uint32_t sum = 0;
  } CMMC_SENSOR_DATA_T;

typedef std::function<void(void *, size_t len)> callback_t;
class CMMC_Sensor {
  public: 
    CMMC_Sensor();
    ~CMMC_Sensor();
    void onData(callback_t cb);
    void every(uint32_t ms);
    virtual void setup(int a = 0, int b = 0)  = 0;
    virtual void read() = 0;
  protected:
    String tag;
    callback_t cb;
    uint32_t everyMs = 5000L;
    CMMC_Interval interval;
    CMMC_SENSOR_DATA_T data;
};
#endif
