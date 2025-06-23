#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Wire.h>
#include <MPU9250_asukiaaa.h>
#include <ArduinoJson.h>

struct SensorData {
  float ax, ay, az;
  float gx, gy, gz;
  int pressure;
  int battery;  
};

class SensorManager {
public:
  void begin();
  SensorData read();
  String toJson(const SensorData& data, bool wifiConnected);

private:
  MPU9250_asukiaaa imu;

  int readFSRAveraged() const;
  int readPressurePercentage() const;
  float calculateGrams(int percentage) const;

  float readBatteryVoltage() const;
  int getBatteryPercent(float voltage) const;
};

#endif
