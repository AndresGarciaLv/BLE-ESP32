#include "WiFiManager.h"
#include "BLEManager.h"
#include <WiFi.h>
#include <Preferences.h>
#include <ArduinoJson.h>

Preferences preferences;

void WiFiManagerCustom::begin() {
  WiFi.begin();  // Inicializa WiFi sin conectarse aún
}

void WiFiManagerCustom::setBLEManager(BLEManager* ble) {
  bleManager = ble;
}

void WiFiManagerCustom::setNotifyCallback(std::function<void(const String&)> callback) {
  notifyCallback = callback;
}

void WiFiManagerCustom::setSendDataControl(std::function<void(bool)> control) {
  sendDataControl = control;
}

void WiFiManagerCustom::connectToWiFi(const char* ssid, const char* pass) {
  Serial.print("🔗 Conectando a: ");
  Serial.println(ssid);

  if (pass && strlen(pass) > 0) {
    WiFi.begin(ssid, pass);
  } else {
    WiFi.begin(ssid);  // Red abierta
  }

  for (int i = 0; i < 20; i++) {
    if (WiFi.status() == WL_CONNECTED) {
      wifiConnected = true;
      bleOnly = false;

      preferences.begin("wifi", false);
      preferences.putString("ssid", ssid);
      preferences.putString("pass", pass ? pass : "");
      preferences.end();

      Serial.println("✅ Conectado a red WiFi correctamente.");

      if (notifyCallback) {
        StaticJsonDocument<128> doc;
        JsonObject status = doc.createNestedObject("wifiStatus");
        status["connected"] = true;
        status["ssid"] = ssid;
        String json;
        serializeJson(doc, json);
        notifyCallback(json);
      }

      return;
    }
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n❌ Error al conectar con WiFi.");
  wifiConnected = false;

 if (notifyCallback) {
  StaticJsonDocument<128> doc;
  JsonObject status = doc.createNestedObject("wifiStatus");
  status["connected"] = false;
  status["ssid"] = ssid;
  String json;
  serializeJson(doc, json);
  notifyCallback(json);
}

}

void WiFiManagerCustom::tryReconnectLastWiFi() {
  preferences.begin("wifi", true);
  String ssid = preferences.getString("ssid", "");
  String pass = preferences.getString("pass", "");
  preferences.end();

  if (ssid.length() > 0) {
    Serial.println("📂 Datos encontrados, intentando reconexión...");

    int n = WiFi.scanNetworks();
    for (int i = 0; i < n; ++i) {
      if (WiFi.SSID(i) == ssid) {
        connectToWiFi(ssid.c_str(), pass.c_str());
        return;
      }
    }

    Serial.println("⚠️ Red guardada no disponible.");
  } else {
    Serial.println("❌ No hay datos guardados para reconexión.");
  }
}

void WiFiManagerCustom::disconnectFromWiFi() {
  WiFi.disconnect(true, true);
  wifiConnected = false;
  bleOnly = true;

  preferences.begin("wifi", false);
  preferences.clear();  // 🧹 Borra SSID y pass
  preferences.end();

  Serial.println("🚫 Desconectado de WiFi y datos limpiados.");

  if (notifyCallback) {
    StaticJsonDocument<96> doc;
    JsonObject status = doc.createNestedObject("wifiStatus");
    status["connected"] = false;
    status["ssid"] = "";
    String json;
    serializeJson(doc, json);
    notifyCallback(json);
  }
}

bool WiFiManagerCustom::isConnected() const {
  return wifiConnected;
}

bool WiFiManagerCustom::isScanning() const {
  return scanning;
}

void WiFiManagerCustom::scanNetworks() {
  Serial.println("📡 Iniciando escaneo de redes WiFi...");

  WiFi.mode(WIFI_STA);  // 🧠 Modo estación
  delay(100);

  // Asegura que no esté conectado a una red
  WiFi.disconnect(true);  // Fuerza desconexión y limpia la configuración
  delay(500);  // Da tiempo a que se estabilice

  int n = WiFi.scanNetworks();
  Serial.printf("📡 Escaneo terminado. Redes encontradas: %d\n", n);

  StaticJsonDocument<2048> doc;
  JsonArray arr = doc.to<JsonArray>();

  if (n <= 0) {
    Serial.println("❌ No se encontraron redes WiFi o error.");
  } else {
    for (int i = 0; i < n; ++i) {
      JsonObject net = arr.createNestedObject();
      String ssid = WiFi.SSID(i);
      int rssi = WiFi.RSSI(i);
      bool open = WiFi.encryptionType(i) == WIFI_AUTH_OPEN;

      Serial.printf("🔍 [%d] SSID: %s | RSSI: %d | %s\n", i + 1, ssid.c_str(), rssi, open ? "Abierta" : "Segura");

      net["ssid"] = ssid;
      net["rssi"] = rssi;
      net["open"] = open;
    }
  }

  String json;
  serializeJson(arr, json);

  if (bleManager) {
    Serial.printf("📤 Enviando JSON WiFi (%d bytes): %s\n", json.length(), json.c_str());
    bleManager->sendFragmented(json);
  } else {
    Serial.println("⚠️ BLEManager no disponible para enviar datos.");
  }
}


void WiFiManagerCustom::notifyWiFiStatus() {
  if (notifyCallback) {
    StaticJsonDocument<128> doc;
    JsonObject status = doc.createNestedObject("wifiStatus");
    status["connected"] = wifiConnected;
    if (wifiConnected) {
      preferences.begin("wifi", true);
      String ssid = preferences.getString("ssid", "");
      preferences.end();
      status["ssid"] = ssid;
    }
    String json;
    serializeJson(doc, json);
    notifyCallback(json);
  }
}

