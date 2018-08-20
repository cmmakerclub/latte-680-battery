#include "MqttModule.h"
extern int temp;

#define MQTT_CONFIG_FILE "/mymqtt.json"
void MqttModule::config(CMMC_System *os, AsyncWebServer* server) {
  strcpy(this->path, "/api/mqtt");
  this->_serverPtr = server;
  this->_managerPtr = new CMMC_ConfigManager(MQTT_CONFIG_FILE);
  this->_managerPtr->init();
  this->_managerPtr->load_config([&](JsonObject * root, const char* content) {
    if (root == NULL) {
      Serial.print("mqtt.json failed. >");
      Serial.println(content);
      return ;
    }
    Serial.println(content);
    Serial.println("[user] mqtt config json loaded..");
    char mqtt_host[40] = "";
    char mqtt_user[40] = "";
    char mqtt_pass[40] = "";
    char mqtt_clientId[40] = "";
    char mqtt_prefix[40] = "";
    char mqtt_port[10] = "";
    char mqtt_device_name[20] = "";
    bool     lwt;
    uint32_t pubEveryS;
    const char* mqtt_configs[] = {(*root)["host"],
                                  (*root)["username"], (*root)["password"],
                                  (*root)["clientId"], (*root)["port"],
                                  (*root)["deviceName"],
                                  (*root)["prefix"], // [6]
                                  (*root)["lwt"],
                                  (*root)["publishRateSecond"]
                                 };

    if (mqtt_configs[0] != NULL) {
      strcpy(mqtt_host, mqtt_configs[0]);
      strcpy(mqtt_user, mqtt_configs[1]);
      strcpy(mqtt_pass, mqtt_configs[2]);
      strcpy(mqtt_clientId, mqtt_configs[3]);
      strcpy(mqtt_port, mqtt_configs[4]);
      strcpy(mqtt_device_name, mqtt_configs[5]);
      strcpy(mqtt_prefix, mqtt_configs[6]);

      lwt = String(mqtt_configs[7]).toInt();
      pubEveryS = String(mqtt_configs[8]).toInt();

      if (strcmp(mqtt_device_name, "") == 0) {
        sprintf(mqtt_device_name, "%08x", ESP.getChipId());
      }
      else {
        Serial.printf("DEVICE NAME = %s\r\n", mqtt_device_name);
      }

      if (strcmp(mqtt_clientId, "") == 0) {
        sprintf(mqtt_clientId, "%08x", ESP.getChipId());
      }
    }
    MQTT_HOST = String(mqtt_host);
    MQTT_USERNAME = String(mqtt_user);
    MQTT_PASSWORD = String(mqtt_pass);
    MQTT_CLIENT_ID = String(mqtt_clientId);
    MQTT_PORT = String(mqtt_port).toInt();
    MQTT_PREFIX = String(mqtt_prefix);
    PUBLISH_EVERY = pubEveryS * 1000L;
    MQTT_LWT = lwt;
    DEVICE_NAME = String(mqtt_device_name);
  });


  this->configWebServer();
};

void MqttModule::configWebServer() {
  static MqttModule *that = this;
  _serverPtr->on(this->path, HTTP_POST, [&](AsyncWebServerRequest * request) {
    String output = that->saveConfig(request, this->_managerPtr);
    request->send(200, "application/json", output);
  });
}

void MqttModule::setup() {
  Serial.println("MqttModule::setup");
  init_mqtt();
};

void MqttModule::loop() {
  mqtt->loop();
};

// MQTT INITIALIZER

MqttConnector* MqttModule::init_mqtt()
{
  this->mqtt = new MqttConnector(this->MQTT_HOST.c_str(), this->MQTT_PORT);

  mqtt->on_connecting([&](int counter, bool * flag) {
    Serial.printf("[%lu] MQTT CONNECTING.. \r\n", counter);
    if (counter >= MQTT_CONNECT_TIMEOUT) {
      ESP.reset();
    }
    delay(1000);
  });

  mqtt->on_prepare_configuration([&](MqttConnector::Config * config) -> void {
    Serial.printf("lwt = %lu\r\n", MQTT_LWT);
    config->clientId  = MQTT_CLIENT_ID;
    config->channelPrefix = MQTT_PREFIX;
    config->enableLastWill = MQTT_LWT;
    config->retainPublishMessage = false;
    /*
        config->mode
        ===================
        | MODE_BOTH       |
        | MODE_PUB_ONLY   |
        | MODE_SUB_ONLY   |
        ===================
    */
    config->mode = MODE_BOTH;
    config->firstCapChannel = false;

    config->username = String(MQTT_USERNAME);
    config->password = String(MQTT_PASSWORD);

    // FORMAT
    // d:quickstart:<type-id>:<device-id>
    //config->clientId  = String("d:quickstart:esp8266meetup:") + macAddr;
    config->topicPub  = MQTT_PREFIX + String(DEVICE_NAME) + String("/status");
  });

  mqtt->on_after_prepare_configuration([&](MqttConnector::Config config) -> void {
    String humanTopic = MQTT_PREFIX + DEVICE_NAME + String("/$/+");
    Serial.printf("[USER] HOST = %s\r\n", config.mqttHost.c_str());
    Serial.printf("[USER] PORT = %d\r\n", config.mqttPort);
    Serial.printf("[USER] PUB  = %s\r\n", config.topicPub.c_str());
    Serial.printf("[USER] SUB  = %s\r\n", config.topicSub.c_str());
    Serial.printf("[USER] SUB  = %s\r\n", humanTopic.c_str());
    // sub->add_topic(MQTT_PREFIX + String("/") + String(myName) + String("#"));
    // sub->add_topic(MQTT_PREFIX + "/" + MQTT_CLIENT_ID + "/$/+");
  });

  if (mqtt == NULL) {
    Serial.println("MQTT is undefined.");
  }

  register_publish_hooks(mqtt);
  register_receive_hooks(mqtt);

  Serial.println("connecting to mqtt..");
  mqtt->connect();
  return mqtt;
}

void MqttModule::register_receive_hooks(MqttConnector *mqtt) {
  mqtt->on_subscribe([&](MQTT::Subscribe * sub) -> void {
    Serial.printf("onSubScribe myName = %s \r\n", DEVICE_NAME.c_str());
    sub->add_topic(MQTT_PREFIX + DEVICE_NAME + String("/$/+"));
    sub->add_topic(MQTT_PREFIX + MQTT_CLIENT_ID + String("/$/+"));
    sub->add_topic(MQTT_PREFIX + DEVICE_NAME + String("/status"));
    Serial.println("done on_subscribe...");
    Serial.printf("publish every %lu s\r\n", PUBLISH_EVERY);
  });

  mqtt->on_before_message_arrived_once([&](void) { });

  mqtt->on_message([&](const MQTT::Publish & pub) { });

  mqtt->on_after_message_arrived([&](String topic, String cmd, String payload) {
    // Serial.printf("recv topic: %s\r\n", topic.c_str());
    // Serial.printf("recv cmd: %s\r\n", cmd.c_str());
    // Serial.printf("payload: %s\r\n", payload.c_str());
    if (cmd == "$/command") {
      if (payload == "ON") {
        Serial.println("ON");
        // gpio.on();
        // relayPinState = 1;
      }
      else if (payload == "OFF") {
        // relayPinState = 0;
        // gpio.off();
        Serial.println("OFF");
      }
      else if (payload == "FORCE_CONFIG") {
        SPIFFS.remove("/enabled");
        ESP.restart();
      }
    }
    else if (cmd == "$/reboot") {
      ESP.restart();
    }
    else if (cmd == "status") {
      // Serial.println("sent & recv.");
    }
    else {
      Serial.println("Another message arrived.");
      // another message.
    }
    // lastRecv = millis();
  });
}

void MqttModule::register_publish_hooks(MqttConnector* mqtt) {
  mqtt->on_prepare_data_once([&](void) {
    Serial.println("initializing sensor...");
  });

  mqtt->on_before_prepare_data([&](void) {
  });

  mqtt->on_prepare_data([&](JsonObject * root) {
    JsonObject& data = (*root)["d"];
    JsonObject& info = (*root)["info"];
    // data["appVersion"] = LEGEND_APP_VERSION;
    data["myName"] = DEVICE_NAME;
    data["millis"] = millis();
    // data["relayPinState"] = relayPinState;
    // data["sensorType"] = sensorType;
    data["updateInterval"] = PUBLISH_EVERY;
    // Serial.printf("field1 = %lu \r\n", sensorData.field1);
    // Serial.printf("field2 = %lu \r\n", sensorData.field2);
    // Serial.printf("field3 = %lu \r\n", sensorData.field3);
    // Serial.printf("field4 = %lu \r\n", sensorData.field4);
    // Serial.printf("field5 = %lu \r\n", sensorData.field5);
    // Serial.printf("field6 = %lu \r\n", sensorData.field6);

    // if (sensorData.field1) { data["field1"] = sensorData.field1; }
    // if (sensorData.field2) { data["field2"] = sensorData.field2; }
    // if (sensorData.field3) { data["field3"] = sensorData.field3; }
    // if (sensorData.field4) { data["field4"] = sensorData.field4; }
    // if (sensorData.field5) { data["field5"] = sensorData.field5; }
    // if (sensorData.field6) { data["field6"] = sensorData.field6; }
    // if (sensorData.field7) { data["field7"] = sensorData.field7; }
    // if (sensorData.field8) { data["field8"] = sensorData.field8; }
    // if (sensorData.ms) { data["ms"] = sensorData.ms; }
    // if (sensorData.battery) { data["battery"] = sensorData.battery; }
    Serial.println("PUBLISHING...!");
    Serial.printf("temp = %d\r\n", temp);

  }, PUBLISH_EVERY);

  mqtt->on_after_prepare_data([&](JsonObject * root) {
    /**************
      JsonObject& data = (*root)["d"];
      data.remove("version");
      data.remove("subscription");
    **************/
  });
}