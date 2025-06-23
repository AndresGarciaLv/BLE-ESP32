#include "SensorManager.h"

#define FSR_PIN 34
#define BATTERY_ADC_PIN 36  // VP / GPIO36

float round2(float val) {
  char buffer[8];
  dtostrf(val, 1, 2, buffer);
  return atof(buffer);
}

// Valor previo suavizado (retiene el último voltaje estimado)
static float previousVoltage = 0.0f;

void SensorManager::begin() {
  Wire.begin();
  imu.setWire(&Wire);
  imu.beginAccel();
  imu.beginGyro();

  analogReadResolution(12);             // 0–4095
  analogSetAttenuation(ADC_11db);       // permite medir hasta ~3.3 V reales
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

// 🔋 Lee voltaje promedio y estabilizado de la batería
float SensorManager::readBatteryVoltage() const {
  const int samples = 10;
  long sum = 0;
  for (int i = 0; i < samples; i++) {
    sum += analogRead(BATTERY_ADC_PIN);
    delay(5);
  }

  float avgADC = sum / (float)samples;
  float voltage = (avgADC / 4095.0f) * 3.3f * 2.0f;  // compensar divisor 100k/100k

  // Filtrado: evitar pequeños saltos (<0.05V)
  if (abs(voltage - previousVoltage) < 0.05f) {
    voltage = previousVoltage;
  } else {
    // Suavizado tipo media móvil: 70% anterior + 30% nuevo
    voltage = (previousVoltage * 0.7f) + (voltage * 0.3f);
  }

  previousVoltage = voltage;
  return voltage;
}

// 🔋 Convertir voltaje a % (rango 3.0V–4.2V)
int SensorManager::getBatteryPercent(float voltage) const {
  float battPercent = (voltage - 3.0f) / (4.2f - 3.0f) * 100.0f;
  return constrain((int)battPercent, 0, 100);
}

SensorData SensorManager::read() {
  imu.accelUpdate();
  imu.gyroUpdate();

  int porcentajePresion = readPressurePercentage();
  float voltajeBateria = readBatteryVoltage();
  int porcentajeBateria = getBatteryPercent(voltajeBateria);

  return {
    imu.accelX(),
    imu.accelY(),
    imu.accelZ(),
    imu.gyroX(),
    imu.gyroY(),
    imu.gyroZ(),
    porcentajePresion,
    porcentajeBateria
  };
}

String SensorManager::toJson(const SensorData& data, bool wifiConnected) {
  StaticJsonDocument<512> doc;
  doc["cmd"] = "save";

  JsonObject d = doc.createNestedObject("data");

  JsonObject p = d.createNestedObject("p");
  p["pc"] = data.pressure;
  p["gr"] = round2(calculateGrams(data.pressure));

  d["b"] = data.battery;  // 🔋 Porcentaje estabilizado

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
