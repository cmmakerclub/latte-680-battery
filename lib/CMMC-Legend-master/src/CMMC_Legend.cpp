#include "CMMC_Legend.h"

void CMMC_Legend::addModule(CMMC_Module* module) {
  _modules.push_back(module);
  Serial.printf("addModule.. size = %d\r\n", _modules.size());
}

void CMMC_Legend::run() {
  static CMMC_Legend *that = this;
  int size = _modules.size();
  for (int i = 0 ; i < size; i++) {
    _modules[i]->loop();
  }
  isLongPressed();
  yield();
}

void CMMC_Legend::isLongPressed() {
  uint32_t prev = millis();
  while (digitalRead(13) == LOW) {
    delay(50);
    if ( (millis() - prev) > 5L * 1000L) {
      Serial.println("LONG PRESSED.");
      blinker->blink(50);
      while (digitalRead(13) == LOW) {
        delay(10);
      }
      SPIFFS.remove("/enabled");
      Serial.println("being restarted.");
      delay(1000);
      ESP.restart();
    }
  }
}
void CMMC_Legend::setup() {
  CMMC_System::setup();
}

void CMMC_Legend::init_gpio() {
  Serial.begin(57600);
  Serial.println("OS::Init GPIO..");
  pinMode(13, INPUT_PULLUP);
  blinker = new CMMC_LED;
  blinker->init();
  blinker->setPin(2);
  Serial.println();
  blinker->blink(500);
  delay(10);
}

void CMMC_Legend::init_fs() {
  // Serial.println("OS::Init FS..");
  SPIFFS.begin();
  Dir dir = SPIFFS.openDir("/");
  isLongPressed();
  // Serial.println("--------------------------");
  // while (dir.next()) {
  //   File f = dir.openFile("r");
  //   Serial.printf("> %s \r\n", dir.fileName().c_str());
  // }
  /*******************************************
     Boot Mode Selection
   *******************************************/
  // Serial.println("--------------------------");
  if (!SPIFFS.exists("/enabled")) {
    mode = SETUP;
  }
  else {
    mode = RUN;
  }
}

void CMMC_Legend::init_user_sensor() {
  Serial.printf("Initializing Sensor.. MODE=%s\r\n", mode == SETUP ? "SETUP" : "RUN");
  if (mode == SETUP) {
    return;
  }
}

void CMMC_Legend::init_user_config() {
  Serial.println("init_user_config");
}

void CMMC_Legend::init_network() {
  Serial.println("Initializing network.");
  for (int i = 0 ; i < _modules.size(); i++) {
    _modules[i]->config(this, &server);
  }
  if (mode == SETUP) {
    _init_ap();
    setupWebServer(&server, &ws, &events);
    blinker->blink(50);
    while (1) {
      for (int i = 0 ; i < _modules.size(); i++) {
        _modules[i]->configLoop();
      }
      yield();
    }
  }
  else if (mode == RUN) {
    blinker->blink(4000);
    for (int i = 0 ; i < _modules.size(); i++) {
      _modules[i]->setup();
    }
  }
}

CMMC_LED *CMMC_Legend::getBlinker() {
  return blinker;
}

void CMMC_Legend::_init_ap() {
  WiFi.disconnect();
  WiFi.softAPdisconnect();
  delay(10);
  WiFi.mode(WIFI_AP);
  delay(10);
  IPAddress Ip(192, 168, 4, 1);
  IPAddress NMask(255, 255, 255, 0);
  WiFi.softAPConfig(Ip, Ip, NMask);
  sprintf(&this->ap_ssid[5], "%08x", ESP.getChipId());
  WiFi.softAP(ap_ssid, &ap_ssid[5]);
  delay(20);
  IPAddress myIP = WiFi.softAPIP();
  Serial.println();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
}
void CMMC_Legend::setupWebServer(AsyncWebServer *server, AsyncWebSocket *ws, AsyncEventSource *events) {
  // ws->onEvent(this->onWsEvent);
  server->addHandler(ws);
  server->serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
  events->onConnect([](AsyncEventSourceClient * client) {
    client->send("hello!", NULL, millis(), 1000);
  });
  server->addHandler(events);
  server->addHandler(new SPIFFSEditor(http_username, http_password));

  server->on("/heap", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", String(ESP.getFreeHeap()));
  });

  server->on("/reboot", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", "OK");
    blinker->blink(20);
    ESP.restart();
  });

  server->on("/enable", HTTP_GET, [](AsyncWebServerRequest * request) {
    File f = SPIFFS.open("/enabled", "a+");
    if (!f) {
      Serial.println("file open failed");
    }
    request->send(200, "text/plain", String("ENABLING.. ") + String(ESP.getFreeHeap()));
    // ESP.restart();
  });

  static const char* fsServerIndex = "<form method='POST' action='/do-fs' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";
  static const char* serverIndex = "<form method='POST' action='/do-firmware' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";
  server->on("/firmware", HTTP_GET, [](AsyncWebServerRequest * request) {
    AsyncWebServerResponse *response = request->beginResponse(200, "text/html", serverIndex);
    response->addHeader("Connection", "close");
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
  });

  server->on("/fs", HTTP_GET, [](AsyncWebServerRequest * request) {
    AsyncWebServerResponse *response = request->beginResponse(200, "text/html", fsServerIndex);
    response->addHeader("Connection", "close");
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
  });

  server->on("/do-fs", HTTP_POST, [](AsyncWebServerRequest * request) {
    // the request handler is triggered after the upload has finished...
    // create the response, add header, and send response
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    response->addHeader("Connection", "close");
    response->addHeader("Access-Control-Allow-Origin", "*");
    // restartRequired = true;  // Tell the main loop to restart the ESP
    request->send(response);
  }, [](AsyncWebServerRequest * request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    //Upload handler chunks in data
    if (!index) { // if index == 0 then this is the first frame of data
      SPIFFS.end();
      blinker->detach();
      Serial.println("upload start...");
      Serial.printf("UploadStart: %s\n", filename.c_str());
      Serial.setDebugOutput(true);
      // calculate sketch space required for the update
      uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
      bool updateOK = maxSketchSpace < ESP.getFreeSketchSpace();
      if (!Update.begin(maxSketchSpace, U_SPIFFS)) { //start with max available size
        Update.printError(Serial);
      }
      Update.runAsync(true); // tell the updaterClass to run in async mode
    }

    //Write chunked data to the free sketch space
    if (Update.write(data, len) != len) {
      Update.printError(Serial);
    }

    if (final) { // if the final flag is set then this is the last frame of data
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u B\nRebooting...\n", index + len);
        blinker->blink(1000);
      } else {
        Update.printError(Serial);
      }
      Serial.setDebugOutput(false);
    }
  });

  server->on("/do-firmware", HTTP_POST, [](AsyncWebServerRequest * request) {
    // the request handler is triggered after the upload has finished...
    // create the response, add header, and send response
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    response->addHeader("Connection", "close");
    response->addHeader("Access-Control-Allow-Origin", "*");
    // restartRequired = true;  // Tell the main loop to restart the ESP
    request->send(response);
  }, [](AsyncWebServerRequest * request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    //Upload handler chunks in data
    if (!index) { // if index == 0 then this is the first frame of data
      blinker->detach();
      SPIFFS.end();
      Serial.println("upload start...");
      Serial.printf("UploadStart: %s\n", filename.c_str());
      Serial.setDebugOutput(true);
      // calculate sketch space required for the update
      uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
      bool updateOK = maxSketchSpace < ESP.getFreeSketchSpace();
      if (!Update.begin(maxSketchSpace)) { //start with max available size
        Update.printError(Serial);
      }
      Update.runAsync(true); // tell the updaterClass to run in async mode
    }

    //Write chunked data to the free sketch space
    if (Update.write(data, len) != len) {
      Update.printError(Serial);
    }

    if (final) { // if the final flag is set then this is the last frame of data
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u B\nRebooting...\n", index + len);
        blinker->blink(1000);
      } else {
        Update.printError(Serial);
      }
      Serial.setDebugOutput(false);
    }
  });

  server->onNotFound([](AsyncWebServerRequest * request) {
    Serial.printf("NOT_FOUND: ");
    if (request->method() == HTTP_GET)
      Serial.printf("GET");
    else if (request->method() == HTTP_POST)
      Serial.printf("POST");
    else if (request->method() == HTTP_DELETE)
      Serial.printf("DELETE");
    else if (request->method() == HTTP_PUT)
      Serial.printf("PUT");
    else if (request->method() == HTTP_PATCH)
      Serial.printf("PATCH");
    else if (request->method() == HTTP_HEAD)
      Serial.printf("HEAD");
    else if (request->method() == HTTP_OPTIONS)
      Serial.printf("OPTIONS");
    else
      Serial.printf("UNKNOWN");
    Serial.printf(" http://%s%s\n", request->host().c_str(), request->url().c_str());

    if (request->contentLength()) {
      Serial.printf("_CONTENT_TYPE: %s\n", request->contentType().c_str());
      Serial.printf("_CONTENT_LENGTH: %u\n", request->contentLength());
    }

    int headers = request->headers();
    int i;
    // for (i = 0; i < headers; i++) {
    //   // AsyncWebHeader* h = request->getHeader(i);
    //   // Serial.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
    // }

    int params = request->params();
    for (i = 0; i < params; i++) {
      AsyncWebParameter* p = request->getParam(i);
      if (p->isFile()) {
        Serial.printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
      } else if (p->isPost()) {
        Serial.printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      } else {
        Serial.printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
    }

    request->send(404);
  });


  server->begin();
  Serial.println("Starting webserver->..");
}
