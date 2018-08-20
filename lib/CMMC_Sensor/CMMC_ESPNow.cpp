#define CMMC_USE_ALIAS

#include "CMMC_ESPNow.h"
#include <CMMC_Utils.h>

extern uint8_t Send_complete;

CMMC_ESPNow::CMMC_ESPNow() {
  static CMMC_ESPNow* that = this;
  this->_user_debug_cb = [](const char* s) { };
  this->_user_on_message_recv = [](uint8_t *macaddr, uint8_t *data, uint8_t len) {};
  this->_user_on_message_sent = [](uint8_t *macaddr, u8 status) {};

  // set system cb
  this->_system_on_message_recv = [](uint8_t *macaddr, uint8_t *data, uint8_t len) {
    that->_user_debug_cb("_system_on_message_recv");
    that->_waiting_message_has_arrived = true;
    that->_user_on_message_recv(macaddr, data, len);
  };

  this->_system_on_message_sent = [](uint8_t *macaddr, u8 status) {
    that->_message_sent_status = status;
    that->_user_on_message_sent(macaddr, status);
  };
}

void CMMC_ESPNow::init(int mode) {
  WiFi.disconnect();
  delay(20);
  if (mode == NOW_MODE_SLAVE) {
    WiFi.mode(WIFI_STA);
  }
  else {
    WiFi.mode(WIFI_STA);
  }

  delay(20); 

  if (esp_now_init() == 0) {
		USER_DEBUG_PRINTF("espnow init ok");
  } else {
		USER_DEBUG_PRINTF("espnow init failed");
    ESP.restart();
    return;
  }

  if (mode == NOW_MODE_CONTROLLER) {
    esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  } else {
    esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  }

  esp_now_register_send_cb(this->_system_on_message_sent);
  esp_now_register_recv_cb(this->_system_on_message_recv);
};


void CMMC_ESPNow::send(uint8_t *mac, u8* data, int len, void_cb_t cb, uint32_t wait_time) {
  if (mac[0] == 0x00 && mac[1] == 0x00) {
    return;
  }
  this->_message_sent_status = -1;
  this->_waiting_message_has_arrived = false;
  uint16_t MAX_RETRIES   = 10;
  uint16_t RETRIES_DELAY = 10;
  uint16_t retries = 0;

  esp_now_send(mac, data, len);
  delay(RETRIES_DELAY*(retries+1));

  if (this->_enable_retries) {
    while(this->_message_sent_status != 0) {
      USER_DEBUG_PRINTF("try to send over espnow..."); 
      esp_now_send(mac, data, len);
      delay(RETRIES_DELAY*(retries+1));
      Serial.printf("retrying %d/%d (at %lums)\r\n", retries, MAX_RETRIES, millis());
      if (++retries > MAX_RETRIES) {
        Serial.printf("reach max retries.\r\n");
        break;
      }
    }
  }

  if (cb != NULL) {
    uint32_t timeout_at_ms = millis() + wait_time;
    USER_DEBUG_PRINTF("timeout at %lu/%lu", millis(), timeout_at_ms);
    while (millis() < timeout_at_ms && this->_waiting_message_has_arrived == false) {
      yield();
    }

    if (this->_waiting_message_has_arrived==false) {
      USER_DEBUG_PRINTF("MESSAGE LOST!, Waiting a message Timeout...\r\n");
      cb();
    }
    else {
      USER_DEBUG_PRINTF("GOT a message from controller\r\n"); 
    }
  }
  else {
    Serial.println("cb is null");
  }
}

void CMMC_ESPNow::on_message_recv(esp_now_recv_cb_t cb) {
  if (cb != NULL) {
    this->_user_on_message_recv = cb;
  }
}

void CMMC_ESPNow::on_message_sent(esp_now_send_cb_t cb) {
  if (cb != NULL) {
    this->_user_on_message_sent = cb;
  }
}

void CMMC_ESPNow::debug(cmmc_debug_cb_t cb) {
  if (cb!=NULL)
    this->_user_debug_cb = cb;
}

void CMMC_ESPNow::enable_retries(bool s) {
  this->_enable_retries = s;
}
