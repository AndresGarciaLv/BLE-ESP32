#include "SensorManager.h"

#define FSR_PIN 34

float round2(float val) {
  char buffer[8];
  dtostrf(val, 1, 2, buffer);
  return atof(buffer);
}

void SensorManager::begin() {
  Wire.begin();
  imu.setWire(&Wire);
  imu.beginAccel();
  imu.beginGyro();

  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
}

int SensorManager::readFSRAveraged() const {
  long sum = 0;
  const int samples = 10;
  for (int i = 0; i < samples; i++) {
    sum += analogRead(FSR_PIN);
    delay(5);
  }
  return sum / samples;
}

int SensorManager::readPressurePercentage() const {
  int raw = readFSRAveraged();
  if (raw < 200) raw = 0;
  int porcentaje = map(raw, 800, 3500, 0, 100);
  return constrain(porcentaje, 0, 100);
}

float SensorManager::calculateGrams(int percentage) const {
  return percentage * 5.0f;
}

SensorData SensorManager::read() {
  imu.accelUpdate();
  imu.gyroUpdate();

  int porcentaje = readPressurePercentage();

  return {
    imu.accelX(),
    imu.accelY(),
    imu.accelZ(),
    imu.gyroX(),
    imu.gyroY(),
    imu.gyroZ(),
    porcentaje,
    random(80, 100)
  };
}

String SensorManager::toJson(const SensorData& data, bool wifiConnected) {
  StaticJsonDocument<512> doc;
  doc["cmd"] = "save";

  JsonObject d = doc.createNestedObject("data");

  JsonObject p = d.createNestedObject("p");
  p["pc"] = data.pressure;
  p["gr"] = round2(calculateGrams(data.pressure));

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
