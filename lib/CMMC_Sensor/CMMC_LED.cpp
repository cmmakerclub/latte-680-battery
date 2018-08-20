#include "CMMC_LED.h"

CMMC_LED CMMC_LED::init(blink_t type)
{
  if (type == BLINK_TYPE_TICKER)
  {
    this->_ticker = new Ticker;
    this->_ticker2 = new Ticker;
  }
  _initialized = true;
  return *this;
}


CMMC_LED::CMMC_LED(blink_t type)
{
  _type = type;
};

CMMC_LED::CMMC_LED(Ticker *ticker)
{
  _initialized = true;
  this->_ticker = ticker;
};

void CMMC_LED::high() {
  digitalWrite(_ledPin, HIGH); 
}

void CMMC_LED::low() {
  digitalWrite(_ledPin, LOW); 
}

void CMMC_LED::setPin(uint8_t pin)
{
  _ledPin = pin;
  pinMode(_ledPin, OUTPUT);
  digitalWrite(_ledPin, LOW);
}

void CMMC_LED::toggle()
{
  this->state = !this->state;
  digitalWrite(this->_ledPin, this->state);
}

void CMMC_LED::blink(uint32_t ms, uint8_t pin)
{
  this->setPin(pin);
  this->blink(ms);
}

void CMMC_LED::detach()
{
  this->_ticker->detach();
  this->_ticker2->detach();
}

void CMMC_LED::blink(uint32_t ms)
{
  if (_initialized == false)
    return;
  if (_ledPin == 254)
    return;
  static int _pin = this->_ledPin;
  this->detach();
  delete this->_ticker;
  delete this->_ticker2;
  this->_ticker = new Ticker;
  this->_ticker2 = new Ticker;
  static CMMC_LED *_that = this;
  static auto lambda = []() {
    _that->state = !_that->state;
    if (_that->state == LOW)
    {
      _that->prev_active = millis();
    }
    digitalWrite(_pin, _that->state);
  };
  static auto wtf = []() {
    uint32_t diff = (millis() - _that->prev_active);
    if (diff > 60L)
    {
      _that->prev_active = millis();
      _that->state = HIGH;
      digitalWrite(_pin, _that->state);
    }
  };
  // auto function  = static_cast<void (*)(int)>(lambda);
  this->_ticker->attach_ms(ms, lambda);
  this->_ticker2->attach_ms(30, wtf);
}