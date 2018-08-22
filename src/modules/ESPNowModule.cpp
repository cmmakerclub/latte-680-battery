#include "ESPNowModule.h"

void printMacAddress(uint8_t* macaddr, uint8_t newline = 0) {
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
  while (len--) {
    sum ^= *(data++);
  }
  return sum;
}

void ESPNowModule::config(CMMC_System *os, AsyncWebServer* server) {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(13, INPUT_PULLUP);
  uint8_t* slave_addr = getESPNowSlaveMacAddress();
  memcpy(self_mac, slave_addr, 6);
  this->led = ((CMMC_Legend*) os)->getBlinker();;
  strcpy(this->path, "/api/espnow");

  static ESPNowModule *that = this;
  this->os = os;
  this->_serverPtr = server;
  this->_managerPtr = new CMMC_ConfigManager("/espnow.json");
  this->_managerPtr->init();

  this->_managerPtr->load_config([](JsonObject * root, const char* content) {
    if (root == NULL) return;
    Serial.println("[user] json loaded..");
    if (root->containsKey("mac")) {
      String macStr = String((*root)["mac"].as<const char*>());
      const char* device = (*root)["deviceName"].as<const char*>();
      String deviceName;
      if (device != NULL) {
        deviceName  = String(device);
        //  strcpy(userEspnowSensorName, deviceName.c_str());
      }
      Serial.printf("Loaded mac %s, name=%s\r\n", macStr.c_str(), deviceName.c_str());
      uint8_t mac[6];
      convertMacStringToUint8(macStr.c_str(), mac);
      memcpy(that->master_mac, mac, 6);
    }
    else {
      Serial.println("no mac field.");
    }
  });
  this->configWebServer();
}

void ESPNowModule::loop() {
  // u8 t = 1;
  if (millis() % 100 == 0) {
    //   espNow.send(master_mac, &t, 1, []() {
    //     Serial.println("espnow sending timeout.");
    //   }, 200);
  }
  // Serial.println("HELLO");
  // delay(10);
}

void ESPNowModule::configLoop() {
  if (digitalRead(BUTTON_PIN) == 0) {
    _init_simple_pair();
    delay(1000);
  }
  if (digitalRead(13) == 0) {
    File f = SPIFFS.open("/enabled", "a+");
    ESP.restart();
  }
}

void ESPNowModule::setup() {
  _init_espnow();
  // memcpy(&userKadyaiData.to, master_mac, 6);
  // userKadyaiData.sum = CMMC::checksum((uint8_t*) &userKadyaiData, sizeof(userKadyaiData) - sizeof(userKadyaiData.sum));
  // CMMC::dump((u8*) &userKadyaiData, sizeof(userKadyaiData));
  // Serial.printf("sending..\r\n");
  // Serial.printf("field1=%ld, field2=%ld, field3=%ld, field4=%ld, field5=%ld\r\n",
  //   userKadyaiData.field1, userKadyaiData.field2, userKadyaiData.field3, userKadyaiData.field4, userKadyaiData.field5);
  // Serial.printf("field6=%ld, field7=%ld, field8=%ld, field9=%ld\r\n",
  //   userKadyaiData.field6, userKadyaiData.field7, userKadyaiData.field8, userKadyaiData.field9);
  // espNow.send(master_mac, (u8*) &userKadyaiData, sizeof(userKadyaiData), [&]() {
  //   Serial.printf("espnow sending timeout. sleepTimeM = %lu\r\n", _defaultDeepSleep_m);
  //   _go_sleep(_defaultDeepSleep_m);
  // }, 1000);
}

void ESPNowModule::_init_espnow() {
  // espNow.debug([](const char* msg) { Serial.println(msg); });
  espNow.init(NOW_MODE_SLAVE);
  espNow.enable_retries(true);
  static CMMC_LED *led;
  static ESPNowModule* module;
  led = ((CMMC_Legend*) os)->getBlinker();
  led->detach();
  espNow.on_message_sent([](uint8_t *macaddr, u8 status) {
    led->toggle();
  });
  module = this;
  espNow.on_message_recv([](uint8_t * macaddr, uint8_t * data, uint8_t len) {
    // user_espnow_sent_at = millis();
    led->toggle();
    Serial.printf("RECV: len = %u byte, sleepTime = %lu at(%lu ms)\r\n", len, data[0], millis());
    module->_go_sleep(data[0]);
  });
}

void ESPNowModule::_init_simple_pair() {
  Serial.println("calling simple pair.");
  this->led->blink(250);
  static ESPNowModule *module = this;
  static bool *flag = &sp_flag_done;
  simplePair.begin(SLAVE_MODE, [](u8 status, u8 * sa, const u8 * data) {
    Serial.println("evt_callback.");
    if (status == 0) {
      char buf[13];
      char self_buf[13];
      Serial.printf("[CSP_EVENT_SUCCESS] STATUS: %d\r\n", status);
      Serial.printf("WITH KEY: ");
      dump(data, 16);
      Serial.printf("WITH MAC: ");
      dump(sa, 6);
      macByteToString(data, buf);
      macByteToString(module->self_mac, self_buf);
      printMacAddress((uint8_t*)buf);
      printMacAddress((uint8_t*)self_buf);
      module->_managerPtr->add_field("mac", buf);
      module->_managerPtr->add_field("self_mac", self_buf);
      module->_managerPtr->commit();
      Serial.println("DONE...");
      *flag = true;
    }
    else {
      Serial.printf("[CSP_EVENT_ERROR] %d: %s\r\n", status, (const char*)data);
    }
  });

  simplePair.start();
  uint32_t startMs = millis();
  while (!sp_flag_done && (millis() - startMs < 10000)) {
    Serial.println("waiting sp_flag_done ..");
    delay(1000L + (250 * sp_flag_done));
  }
  if (sp_flag_done) {
    module->led->blink(1000);
    delay(5000);
    ESP.restart();
    Serial.println("pair done.");
  }
  else {
    Serial.println("do simple pair device not found.");
    module->led->blink(50);
  }
}

void ESPNowModule::_go_sleep(uint32_t deepSleepM) {
  // deepSleepM = 1;
  Serial.printf("\r\nGo sleep for %lu min.\r\n", deepSleepM);
  Serial.println("bye!");

  ESP.deepSleep(deepSleepM * 60e6);
  Serial.println("not be reached here.");
}
