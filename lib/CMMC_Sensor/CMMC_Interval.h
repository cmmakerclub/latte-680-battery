#ifndef CMMCVTimer_h
#define CMMCVTimer_h
#include <Arduino.h>
#include <functional>

class CMMC_Interval
{
  private:
    unsigned long _prev;
    unsigned long _now;
    unsigned long _threshold_ms;

	public:
    typedef std::function<void(void)> void_callback_t;
    CMMC_Interval();
    ~CMMC_Interval();
    void every_ms(unsigned long ms, void_callback_t cb);

};


#endif
