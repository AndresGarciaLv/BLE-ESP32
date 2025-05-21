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

bool wasWiFiConnected = false;
String wsUrl;

const float ACC_THRESHOLD = 200.0;
const float GYRO_THRESHOLD = 2000.0;

void checkSafetyLimits(const SensorData& data) {
  if (isnan(data.ax) || isnan(data.ay) || isnan(data.az) ||
      isnan(data.gx) || isnan(data.gy) || isnan(data.gz)) {
    Serial.println("❌ Sensor NaN. Reiniciando...");
    delay(2000);
    ESP.restart();
  }

  if (abs(data.ax) > ACC_THRESHOLD || abs(data.ay) > ACC_THRESHOLD || abs(data.az) > ACC_THRESHOLD ||
      abs(data.gx) > GYRO_THRESHOLD || abs(data.gy) > GYRO_THRESHOLD || abs(data.gz) > GYRO_THRESHOLD) {
    Serial.println("🚨 Movimiento extremo. Reiniciando...");
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

  // Mostrar MAC
  String mac = WiFi.macAddress();
  Serial.println("🆔 Dirección MAC del peluche: " + mac);

  // Construir URL con MAC
  wsUrl = "ws://192.168.1.6:5002/ws/sensor-data?device=esp32&identifier=" + mac;

  Serial.println("✅ PelucheSmart iniciado");
}

void loop() {
  wsManager.poll();

  bool wifiNow = wifiManager.isConnected();

  // Conectar WebSocket si recién obtuvo WiFi
  if (wifiNow && !wasWiFiConnected) {
    Serial.println("🔌 Conectando al WebSocket...");
    wsManager.connect(wsUrl.c_str());
  }
  wasWiFiConnected = wifiNow;

  if (!wifiManager.isScanning()) {
    SensorData data = sensorManager.read();
    checkSafetyLimits(data);

    String json = sensorManager.toJson(data, wifiNow);

    if (json.startsWith("{") && json.endsWith("}")) {
      if (wifiNow && wsManager.isAvailable()) {
        wsManager.send(json);
      } else if (bleManager.isDeviceConnected() && wifiManager.isBLEOnly()) {
        bleManager.sendFragmented(json);
      }
    } else {
      Serial.println("⚠️ JSON mal formado.");
    }
  }

  delay(100);
}
