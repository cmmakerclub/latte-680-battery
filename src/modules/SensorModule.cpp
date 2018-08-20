#include "SensorModule.h" 

#include <CMMC_BME680.hpp>
#include <CMMC_BME280.hpp>
#include <CMMC_DHT.hpp>
#include <map>
#include <functional>


#define SENSOR_CONFIG_FILE "/sensors.json"
static CMMC_ConfigManager *m2 = new CMMC_ConfigManager(SENSOR_CONFIG_FILE);

void SensorModule::config(CMMC_System *os, AsyncWebServer* server) {
  strcpy(this->path, "/api/sensors");
  static SensorModule *that = this;
  this->_serverPtr = server;
  this->_managerPtr = m2;
  this->_managerPtr->init(); 
  std::map<String, std::function<CMMC_Sensor*()>> factory {
    {"BME680", []() { return new CMMC_BME680(); } },
    {"BME280", []() { return new CMMC_BME280(); } },
    {"DHT11", []() { return new CMMC_DHT(11, 12); } },
    {"DHT22", []() { return new CMMC_DHT(22, 12); } }
};

  this->_managerPtr->load_config([&](JsonObject * root, const char* content) {
    if (root == NULL) {
      Serial.print("sensor.json failed. >");
      Serial.println(content);
      return ;
    }
    Serial.println("[user] sensor config json loaded..");
    const char* config[2];
    config[0] = (*root)["sensorType"];
    strcpy(sensorName, String(config[0]).c_str()); 
  });
  String s = String(sensorName);
  // Serial.println(s);
  // sensor = factory[s];
  if (factory[s] == NULL) {
    Serial.println("NULL");
  }
  else {
    Serial.println("initializing sensor..");
    this->sensor = factory[s](); 
    sensor->setup();
    sensor->every(3000);
    sensor->onData([](void *d, size_t len) {
      // memcpy(&data, d, len);
      Serial.printf("ON SENSOR DATA.. at %lums\r\n", millis());
    });
  }
  this->configWebServer();
}

void SensorModule::configWebServer() {
  static SensorModule *that = this;
  _serverPtr->on(this->path, HTTP_POST, [&](AsyncWebServerRequest * request) {
    String output = that->saveConfig(request, m2);
    request->send(200, "application/json", output);
  }); 
}
void SensorModule::setup() {
}

void SensorModule::loop() {
  if (sensor) {
    sensor->read();
  } 
}
