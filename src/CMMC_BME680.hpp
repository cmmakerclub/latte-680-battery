#ifndef CMMC_SENSOR_BME680_H
#define CMMC_SENSOR_BME680_H

#include <Arduino.h>
#include <CMMC_Interval.h>
#include <bsec.h>

// Helper functions declarations
void checkIaqSensorStatus(void);
void errLeds(void);

#define SEALEVELPRESSURE_HPA (1013.25)
#define CMMC_BME680_MAX_DATA_ARRAY (5)

class CMMC_BME680
{
  private:
    uint32_t _gas_data[CMMC_BME680_MAX_DATA_ARRAY] = {0.0};
    
    uint32_t startMeasurementAtMs;
    uint8_t measurementIdx = 0;
    uint32_t measurementCounter = 0;

    Bsec iaqSensor;
    String output;

    CMMC_Interval readSensorInterval;

    float temperature;
    float hudmidity;
    float iaqEstimate;
    float iaqAccuracy;
    float rawTemperature;
    float rawHumidity;
    float ms;

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
      Serial.println("680 constructure.");
    }

    ~CMMC_BME680()
    {
      Serial.println("680 destructure.");
    }


    void setup()
    {
      Serial.println("BME680 begin..");
      Serial.println("BME680 initialized.");
      this->startMeasurementAtMs = millis();
      iaqSensor.begin(0x77, Wire); 
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
      this->readSensorInterval.every_ms(1000, [&]() {
          if (!isBmeError && iaqSensor.run())
          { // If new data is available
            Serial.printf("temp=%f, rawTemp=%f, humidity=%f, raw_humididy=%f, iaq=%f, iaqAcc=%u\r\n",
                          iaqSensor.temperature, iaqSensor.rawTemperature,
                          iaqSensor.humidity, iaqSensor.rawHumidity,
                          iaqSensor.iaqEstimate, iaqSensor.iaqAccuracy);
            temperature = iaqSensor.temperature;
            hudmidity = iaqSensor.humidity;
            iaqEstimate = (iaqSensor.iaqEstimate) + 1;
            iaqAccuracy = iaqSensor.iaqAccuracy;
            rawTemperature = iaqSensor.rawTemperature * 100;
            rawHumidity = iaqSensor.rawHumidity * 100;
            ms = millis();
          }
          else
          {
            // Serial.println("BME data not available.");
            checkIaqSensorStatus();
          }
      });
    };

    public:
      float getTemperature() {
        return temperature;
      }

      float getHumidity() {
        return hudmidity;
      }
};
#endif