#include "BLEManager.h"
#include "WiFiManager.h"
#include "WebSocketManager.h"
#include "SensorManager.h"
#include "CommandParser.h"

BLEManager bleManager;
WiFiManagerCustom wifiManager;
WebSocketManager wsManager;
SensorManager sensorManager;
CommandParser cmdParser;

// Umbral de seguridad en G y grados/segundo
const float ACC_THRESHOLD = 200.0;  // aceleración extrema
const float GYRO_THRESHOLD = 2000.0;  // velocidad angular extrema

void checkSafetyLimits(const SensorData& data) {
  if (isnan(data.ax) || isnan(data.ay) || isnan(data.az) ||
      isnan(data.gx) || isnan(data.gy) || isnan(data.gz)) {
    Serial.println("❌ Datos del sensor inválidos (NaN). Reiniciando...");
    delay(2000);
    ESP.restart();
  }

  if (abs(data.ax) > ACC_THRESHOLD || abs(data.ay) > ACC_THRESHOLD || abs(data.az) > ACC_THRESHOLD ||
      abs(data.gx) > GYRO_THRESHOLD || abs(data.gy) > GYRO_THRESHOLD || abs(data.gz) > GYRO_THRESHOLD) {
    Serial.println("🚨 Movimiento extremo detectado. Reiniciando por seguridad...");
    delay(2000);
    ESP.restart();
  }
}

void setup() {
  Serial.begin(115200);
  bleManager.begin();
  wifiManager.begin();
  sensorManager.begin();
  cmdParser.setManagers(&wifiManager);
  bleManager.setCommandParser(&cmdParser);

  Serial.println("✅ PelucheSmart iniciado correctamente");
}

void loop() {
  wsManager.poll();

  if (!wifiManager.isScanning()) {
    SensorData data = sensorManager.read();

    // 🛡️ Protección contra cuelgues por valores extremos
    checkSafetyLimits(data);

    String json = sensorManager.toJson(data, wifiManager.isConnected());

    // Debug
    Serial.println("📦 JSON generado:");
    Serial.println(json);

    // Enviar datos si el JSON es válido
    if (json.startsWith("{") && json.endsWith("}")) {
      if (wifiManager.isConnected() && wsManager.isAvailable()) {
        wsManager.send(json);
      } else if (bleManager.isDeviceConnected() && wifiManager.isBLEOnly()) {
        bleManager.sendFragmented(json);
      }
    } else {
      Serial.println("⚠️ JSON mal formado. Se omitió el envío.");
    }
  }

  delay(100);
}
