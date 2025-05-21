#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Wire.h>
#include <MPU9250_asukiaaa.h>
#include <ArduinoJson.h>

struct SensorData {
  float ax, ay, az;
  float gx, gy, gz;
  int pressure;
  int bpm;
};

class SensorManager {
public:
  void begin();
  SensorData read();
  String toJson(const SensorData& data, bool wifiConnected);

private:
  MPU9250_asukiaaa imu;
};

#endif
