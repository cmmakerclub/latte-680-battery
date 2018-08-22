
#ifndef CMMC_ESPNOW_MODULE_H
#define CMMC_ESPNOW_MODULE_H
#define CMMC_USE_ALIAS 

#include <CMMC_Legend.h>
#include <CMMC_Module.h>
#include <CMMC_ESPNow.h>
#include <CMMC_SimplePair.h>
#include <CMMC_LED.h>

#define BUTTON_PIN  0

class ESPNowModule: public CMMC_Module {
  public:
    void config(CMMC_System *os, AsyncWebServer* server); 
    void configLoop(); 
    void setup(); 
    void loop(); 
  private:
    uint8_t _defaultDeepSleep_m = 30;
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
}; 

 void printMacAddress(uint8_t* macaddr, uint8_t newline=0) {
      Serial.print("{");
      for (int i = 0; i < 6; i++) {
        Serial.print("0x");
        Serial.print(macaddr[i], HEX);
        if (i < 5) Serial.print(',');
      }
      Serial.println("};");
    }

void dump(const u8* data, size_t size) {
  for (size_t i = 0; i < size; i++) {
    Serial.printf("%02x ", data[i]);
  }
  Serial.println();
}

void convertMacStringToUint8(const char* mac_str, uint8_t* target) {
  String macStr = String(mac_str);
  for (size_t i = 0; i < 12; i += 2) {
    String mac = macStr.substring(i, i + 2);
    byte b = strtoul(mac.c_str(), 0, 16);
    target[i / 2] = b;
  }
}

void macByteToString(const u8* data, char *target) {
    bzero(target, 13);
    sprintf(target, "%02x%02x%02x%02x%02x%02x", data[0], data[1], data[2], data[3], data[4], data[5]);
  }

  uint8_t* getESPNowControllerMacAddress() {
    uint8_t _controller_macaddr[6];
    bzero(_controller_macaddr, 6);
    wifi_get_macaddr(STATION_IF, _controller_macaddr);
    return _controller_macaddr;
  }

  uint8_t* getESPNowSlaveMacAddress() {
    uint8_t _slave_macaddr[6];
    bzero(_slave_macaddr, 6);
    wifi_get_macaddr(STATION_IF, _slave_macaddr);
    return _slave_macaddr;
  }

uint32_t checksum(uint8_t* data, size_t len) {
  uint32_t sum = 0;
  while(len--) {
    sum ^= *(data++);
  }
  return sum;
}

#endif