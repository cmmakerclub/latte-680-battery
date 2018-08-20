#include "ESPNowModule.h"

extern char userEspnowSensorName[16];

// extern uint32_t user_espnow_sent_at; 
// void printBits(size_t const size, void const * const ptr) {
//     unsigned char *b = (unsigned char*) ptr;
//     unsigned char byte;
//     int i, j;

//     for (i=0; i<= size-1;i++) {
//         for (j=7;j>=0;j--)
//         {
//             byte = (b[i] >> j) & 1;
//             Serial.printf("%u", byte);
//         }
//         Serial.println();
//     }
//     Serial.println("---");
// }

void ESPNowModule::config(CMMC_System *os, AsyncWebServer* server) {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  uint8_t* slave_addr = CMMC::getESPNowSlaveMacAddress();
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
         strcpy(userEspnowSensorName, deviceName.c_str());
      }
      Serial.printf("Loaded mac %s, name=%s\r\n", macStr.c_str(), deviceName.c_str());
      uint8_t mac[6];
      CMMC::convertMacStringToUint8(macStr.c_str(), mac);
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
  Serial.println("HELLO");
  delay(10);
}

void ESPNowModule::configLoop() {
  if (digitalRead(BUTTON_PIN) == 0) {
    _init_simple_pair();
    delay(1000);
  }
}

extern CMMC_SENSOR_DATA_T userKadyaiData;
void ESPNowModule::setup() { 
  _init_espnow(); 
  memcpy(&userKadyaiData.to, master_mac, 6);
  userKadyaiData.sum = CMMC::checksum((uint8_t*) &userKadyaiData, sizeof(userKadyaiData) - sizeof(userKadyaiData.sum)); 
  CMMC::dump((u8*) &userKadyaiData, sizeof(userKadyaiData));
  Serial.printf("sending..\r\n");
  Serial.printf("field1=%lu, field2=%lu, field3=%lu, field4=%lu, field5=%lu\r\n", userKadyaiData.field1, userKadyaiData.field2, 
  userKadyaiData.field3, userKadyaiData.field4, userKadyaiData.field5);
  espNow.send(master_mac, (u8*) &userKadyaiData, sizeof(userKadyaiData), [&]() {
    Serial.printf("espnow sending timeout. sleepTimeM = %lu\r\n", _defaultDeepSleep_m); 
    _go_sleep(_defaultDeepSleep_m);
  }, 500); 
} 

void ESPNowModule::_init_espnow() {
  // espNow.debug([](const char* msg) { Serial.println(msg); });
  espNow.init(NOW_MODE_SLAVE); 
  espNow.enable_retries(true);

  static CMMC_LED *led;
  led = ((CMMC_Legend*) os)->getBlinker();
  led->detach();
  espNow.on_message_sent([](uint8_t *macaddr, u8 status) { led->toggle(); }); 
  static ESPNowModule* module; 
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
  // simplePair.debug([](const char* msg) { Serial.println(msg); });
  static ESPNowModule *module = this;
  static bool *flag = &sp_flag_done;
  simplePair.begin(SLAVE_MODE, [](u8 status, u8 * sa, const u8 * data) {
    Serial.println("evt_callback.");
    if (status == 0) {
      char buf[13];
      char self_buf[13];
      Serial.printf("[CSP_EVENT_SUCCESS] STATUS: %d\r\n", status);
      Serial.printf("WITH KEY: ");
      CMMC::dump(data, 16);
      Serial.printf("WITH MAC: ");
      CMMC::dump(sa, 6);
      CMMC::macByteToString(data, buf);
      CMMC::macByteToString(module->self_mac, self_buf);
      CMMC::printMacAddress((uint8_t*)buf);
      CMMC::printMacAddress((uint8_t*)self_buf);
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
