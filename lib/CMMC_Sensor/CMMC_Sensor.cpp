#include "CMMC_Sensor.h"

CMMC_Sensor::CMMC_Sensor() {
  cb = [](void* d, uint32_t len) { };
}

void CMMC_Sensor::onData(callback_t cb) {
  this->cb = cb;
}

void CMMC_Sensor::every(uint32_t ms) {
  this->everyMs = ms;
}