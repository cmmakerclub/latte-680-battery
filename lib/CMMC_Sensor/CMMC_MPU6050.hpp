#ifndef CMMC_SENSOR_MPU6050_H
#define CMMC_SENSOR_MPU6050_H

#include <Arduino.h>
#include <CMMC_Sensor.h>
#include <Wire.h>

#define A_R 16384.0 // 32768 / 2
#define G_R 131.0   // 32768 / 250


class CMMC_MPU6050 : public CMMC_Sensor
{
public:
  int16_t AcX, AcY, AcZ, GyX, GyY, GyZ;
  float Acc[2];
  float Gy[3];
  float Angle[3];
  float GyroX, GyroY, GyroZ;

  long time_prev;
  float dt;

  CMMC_MPU6050()
  {
    this->data.type = 0;
  }

  void readMPU()
  {
    Wire.beginTransmission(0x68);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(0x68, 6, true);

    AcX = Wire.read() << 8 | Wire.read();
    AcY = Wire.read() << 8 | Wire.read();
    AcZ = Wire.read() << 8 | Wire.read();

    Acc[1] = atan(-1 * (AcX / A_R) / sqrt(pow((AcY / A_R), 2) + pow((AcZ / A_R), 2))) * RAD_TO_DEG;
    Acc[0] = atan((AcY / A_R) / sqrt(pow((AcX / A_R), 2) + pow((AcZ / A_R), 2))) * RAD_TO_DEG;

    // readMPU();
    Serial.printf("Gx : %2f     Gy : %2f\r\n", Acc[1], Acc[0]);

    data.field1 = abs(Acc[1]) * 100;
    data.field2 = abs(Acc[0]) * 100;
  }

  void setup()
  {
    setup(0, 0);
  }

  // set default I2C Pin on 4 (SDA) and 5 (SCL) from ESPresso Lite V2.0
  void setup(int a = 0, int b = 0)
  {
    Serial.println("MPU6050 begin..");
    Wire.begin(a, b);
    Wire.beginTransmission(0x68);
    Wire.write(0x6B);
    Wire.write(0);
    Wire.endTransmission(true);
    readMPU();
  };

  void read()
  {
    this->interval.every_ms(everyMs, [&]() {
      readMPU();
      cb((void *)&data, sizeof(data));
    });
  };
};

#endif