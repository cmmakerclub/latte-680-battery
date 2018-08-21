
#include <CMMC_Legend.h>
#include <CMMC_BME680.hpp>
#include "modules/LatteModule.h"
#include "modules/WiFiModule.h"
#include "modules/MqttModule.h"

CMMC_Legend os;
CMMC_BME680 *bme;

void setup()
{
  os.addModule(new WiFiModule());
  os.addModule(new LatteModule());
  os.addModule(new MqttModule());
  os.setup();
  Serial.printf("APP VERSION: %s\r\n", LEGEND_APP_VERSION);
}

void loop()
{
  os.run();
}