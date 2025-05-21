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
  doc["cmd"] = "save";  // Se puede parametrizar más adelante

  JsonObject d = doc.createNestedObject("data");

  JsonObject p = d.createNestedObject("p");
  p["pc"] = data.pressure;
  p["gr"] = data.pressure + 160; // Simulado, puedes cambiarlo

  d["b"] = data.bpm;

  JsonObject m = d.createNestedObject("m");
  m["x"] = round2(data.ax);
  m["y"] = round2(data.ay);
  m["z"] = round2(data.az);

  JsonObject g = d.createNestedObject("g");
  g["x"] = round2(data.gx);
  g["y"] = round2(data.gy);
  g["z"] = round2(data.gz);

  d["w"] = wifiConnected;

  String out;
  serializeJson(doc, out);
  return out;
}

