#include "CommandParser.h"
#include "WiFiManager.h"
#include <ArduinoJson.h>

void CommandParser::setManagers(WiFiManagerCustom* wifi) {
  wifiManager = wifi;
}

void CommandParser::handleCommand(const String& input) {
  Serial.println("📩 Comando base64 recibido por BLE:");
  Serial.println("📤 Comando JSON decodificado:");
  Serial.println(input);

  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, input);

  if (error) {
    Serial.println("❌ Error de parseo JSON");
    return;
  }

  // 🔍 Escanear redes WiFi
  if (doc.containsKey("scan") && doc["scan"] == true) {
    if (wifiManager) {
      Serial.println("📡 Iniciando escaneo de redes WiFi...");
      wifiManager->scanNetworks();
    } else {
      Serial.println("⚠️ WiFiManager no disponible para escaneo.");
    }
    return;
  }

  // 🔐 Conectar a red WiFi
  if (doc.containsKey("connect")) {
    if (!wifiManager) {
      Serial.println("⚠️ WiFiManager no disponible para conexión.");
      return;
    }

    JsonObject conn = doc["connect"];
    const char* ssid = conn["ssid"];
    const char* pass = conn["pass"];

    if (ssid) {
      Serial.print("🔐 Intentando conexión a: ");
      Serial.println(ssid);
      wifiManager->connectToWiFi(ssid, pass);
    } else {
      Serial.println("❌ SSID no proporcionado.");
    }
    return;
  }

  // 🔌 Desconectar de red WiFi
  if (doc.containsKey("disconnect") && doc["disconnect"] == true) {
    if (wifiManager) {
      Serial.println("🚫 Desconectando de red WiFi...");
      wifiManager->disconnectFromWiFi();
    } else {
      Serial.println("⚠️ WiFiManager no disponible para desconexión.");
    }
    return;
  }

  // 📶 Estado actual de conexión WiFi
  if (doc.containsKey("status") && doc["status"] == true) {
    if (wifiManager) {
      Serial.println("📶 Enviando estado actual de WiFi...");
      wifiManager->notifyWiFiStatus();
    } else {
      Serial.println("⚠️ WiFiManager no disponible para estado.");
    }
    return;
  }

  Serial.println("⚠️ Comando JSON no reconocido.");
}
