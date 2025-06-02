#include "BLEManager.h"
#include "WiFiManager.h"
#include "WebSocketManager.h"
#include "SensorManager.h"
#include "CommandParser.h"
#include <ArduinoJson.h>

BLEManager bleManager;
WiFiManagerCustom wifiManager;
WebSocketManager wsManager;
SensorManager sensorManager;
CommandParser cmdParser;

bool wasWiFiConnected = false;
String wsUrl;
bool sendData = true;

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

void handleWebSocketCommand(const String& msg) {
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, msg);
  if (error) {
    Serial.println("❌ Error al parsear JSON recibido.");
    return;
  }

  String cmd = doc["cmd"];
  String targetMac = doc["data"]["macAddress"];
  String localMac = WiFi.macAddress();

  if (targetMac != localMac) {
    Serial.println("🔕 Comando para otro dispositivo. Ignorado.");
    return;
  }

  if (cmd == "switchSendingStatus") {
    sendData = doc["data"]["sendData"];
    Serial.print("📶 Envío de datos actualizado a: ");
    Serial.println(sendData ? "ENVIANDO ✅" : "DETENIDO ❌");

  } else if (cmd == "getDataSendingStatus") {
    String userId = doc["data"]["userId"];

    StaticJsonDocument<256> response;
    response["cmd"] = "getDataSendingStatus";
    JsonObject data = response.createNestedObject("data");
    data["userId"] = userId;
    data["macAddress"] = localMac;
    data["enable"] = sendData;

    String out;
    serializeJson(response, out);
    wsManager.send(out);
    Serial.println("🔁 Estado actual reportado al backend.");
  }
}

void setup() {
  Serial.begin(115200);

  bleManager.begin();
  wifiManager.begin();
  wifiManager.tryReconnectLastWiFi();
  sensorManager.begin();

  cmdParser.setManagers(&wifiManager);
  bleManager.setCommandParser(&cmdParser);

  String mac = WiFi.macAddress();
  Serial.println("🆔 Dirección MAC del peluche: " + mac);

  wsUrl = "ws://192.168.1.6:5002/ws/sensor-data?device=esp32&identifier=" + mac;

  // Escuchar comandos entrantes por WebSocket
  wsManager.setOnMessageCallback(handleWebSocketCommand);

  Serial.println("✅ PelucheSmart iniciado");
}

void loop() {
  wsManager.poll();

  bool wifiNow = wifiManager.isConnected();

  if (wifiNow && !wasWiFiConnected) {
    Serial.println("🔌 Conectando al WebSocket...");
    wsManager.connect(wsUrl.c_str());
  }
  wasWiFiConnected = wifiNow;

  if (!wifiManager.isScanning() && sendData) {
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
