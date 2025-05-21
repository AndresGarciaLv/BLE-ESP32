#include "SensorManager.h"

float round2(float val) {
  char buffer[8];
  dtostrf(val, 1, 2, buffer);  // siempre genera -0.12, 0.00, etc.
  return atof(buffer);
}



void SensorManager::begin() {
  Wire.begin();
  imu.setWire(&Wire);
  imu.beginAccel();
  imu.beginGyro();
}

SensorData SensorManager::read() {
  imu.accelUpdate();
  imu.gyroUpdate();

  return {
    imu.accelX(),
    imu.accelY(),
    imu.accelZ(),
    imu.gyroX(),
    imu.gyroY(),
    imu.gyroZ(),
    random(100, 200),
    random(80, 100)
  };
}

String SensorManager::toJson(const SensorData& data, bool wifiConnected) {
  StaticJsonDocument<512> doc;
  doc["p"] = data.pressure;
  doc["b"] = data.bpm;

  JsonObject m = doc.createNestedObject("m");
  m["x"] = round2(data.ax);
  m["y"] = round2(data.ay);
  m["z"] = round2(data.az);

  JsonObject g = doc.createNestedObject("g");
  g["x"] = round2(data.gx);
  g["y"] = round2(data.gy);
  g["z"] = round2(data.gz);

  doc["wifi"] = wifiConnected;

  String out;
  serializeJson(doc, out);
  return out;
}
