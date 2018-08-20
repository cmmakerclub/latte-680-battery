#ifndef CMMC_SENSOR_BME680_H
#define CMMC_SENSOR_BME680_H

#include <Arduino.h>
#include <CMMC_Sensor.h>
#include <bsec.h>

// Helper functions declarations
void checkIaqSensorStatus(void);
void errLeds(void);

#define SEALEVELPRESSURE_HPA (1013.25)
#define CMMC_BME680_MAX_DATA_ARRAY (50)

class CMMC_BME680 : public CMMC_Sensor
{
private:
  uint32_t _gas_data[CMMC_BME680_MAX_DATA_ARRAY] = {0.0};
  CMMC_Interval readSensorInterval;
  uint32_t startMeasurementAtMs;
  uint8_t measurementIdx = 0;
  uint32_t measurementCounter = 0;
  Bsec iaqSensor;
  String output;
  bool isBmeError = false;
  void checkIaqSensorStatus(void)
  {
    if (iaqSensor.status != BSEC_OK)
    {
      if (iaqSensor.status < BSEC_OK)
      {
        output = "BSEC error code : " + String(iaqSensor.status);
        Serial.println(output);
        isBmeError = true;
      }
      else
      {
        output = "BSEC warning code : " + String(iaqSensor.status);
        Serial.println(output);
      }
    }
    else
    {
      isBmeError = false;
    }

    if (iaqSensor.bme680Status != BME680_OK)
    {
      isBmeError = true;
      if (iaqSensor.bme680Status < BME680_OK)
      {
        output = "BME680 error code : " + String(iaqSensor.bme680Status);
        Serial.println(output);
      }
      else
      {
        output = "BME680 warning code : " + String(iaqSensor.bme680Status);
        Serial.println(output);
      }
    }
    else
    {
      isBmeError = false;
    }
  }

public:
  CMMC_BME680()
  {
    this->tag = "BME680";
    this->data.type = 0x02;
    Serial.println("680 constructure.");
  }
  ~CMMC_BME680()
  {
    Serial.println("680 destructure.");
  }

  void setup()
  {
    setup(0, 0);
  }

  void setup(int a = 0, int b = 0)
  {
    Serial.println("BME680 begin..");
    Serial.println("BME680 initialized.");
    this->startMeasurementAtMs = millis();
    iaqSensor.begin(0x77, Wire);
    output = "\nBSEC library version " + String(iaqSensor.version.major) + "." + 
      String(iaqSensor.version.minor) + "." + String(iaqSensor.version.major_bugfix) + 
      "." + String(iaqSensor.version.minor_bugfix);

    Serial.println(output);
    checkIaqSensorStatus();

    bsec_virtual_sensor_t sensorList[7] = {
        BSEC_OUTPUT_RAW_TEMPERATURE,
        BSEC_OUTPUT_RAW_PRESSURE,
        BSEC_OUTPUT_RAW_HUMIDITY,
        BSEC_OUTPUT_RAW_GAS,
        BSEC_OUTPUT_IAQ_ESTIMATE,
        BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
        BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
    };

    iaqSensor.updateSubscription(sensorList, 7, BSEC_SAMPLE_RATE_LP);
    checkIaqSensorStatus();
  };

  void read()
  {
    if (!isBmeError && iaqSensor.run())
    { // If new data is available
      // Serial.printf("temp=%f, rawTemp=%f, humidity=%f, raw_humididy=%f, iaq=%f, iaqAcc=%u\r\n",
      //   iaqSensor.temperature, iaqSensor.rawTemperature,
      //   iaqSensor.humidity, iaqSensor.rawHumidity,
      //   iaqSensor.iaqEstimate, iaqSensor.iaqAccuracy);
      data.field1 = iaqSensor.temperature * 100;
      data.field2 = iaqSensor.humidity * 100;
      data.field3 = (iaqSensor.iaqEstimate * 100) + 1;
      data.field4 = iaqSensor.iaqAccuracy;
      data.field5 = iaqSensor.rawTemperature * 100;
      data.field6 = iaqSensor.rawHumidity * 100;
      data.ms = millis();
    }
    else
    {
      // Serial.println("BME data not available.");
      checkIaqSensorStatus();
    }

    static CMMC_BME680 *that = this;

    that->interval.every_ms(that->everyMs, []() {
      that->cb((void *)&that->data, sizeof(that->data));
    });
  };
};
#endif