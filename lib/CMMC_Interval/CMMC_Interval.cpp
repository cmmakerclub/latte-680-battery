#include <CMMC_Interval.h>

    CMMC_Interval::CMMC_Interval() {
      _prev = 0;
      _now  = millis();
    };

    CMMC_Interval::~CMMC_Interval() {
    };

    void CMMC_Interval::every_ms(unsigned long ms, void_callback_t cb) {
      _threshold_ms = ms;
      _now  = millis();
      unsigned long diff = _now - _prev;
      if (diff >= _threshold_ms) {
        _prev = millis();
        cb();
      }
    }