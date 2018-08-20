#include <CMMC_Gpio.h>

    void CMMC_Gpio::setup() {
      pinMode(15, OUTPUT);
      digitalWrite(15, HIGH);
    }

    void CMMC_Gpio::on() {
      digitalWrite(15, HIGH); 
    }
    
    void CMMC_Gpio::off() {
      digitalWrite(15, LOW); 
    }

