#include "WiFiManager.h"

Preferences preferences;

void WiFiManagerCustom::begin() {
  WiFi.begin();  // sin ssid/pass, solo activa WiFi
}

void WiFiManagerCustom::connectToWiFi(const char* ssid, const char* pass) {
  Serial.print("🔗 Conectando a: ");
  Serial.println(ssid);

  if (pass && strlen(pass) > 0) {
    WiFi.begin(ssid, pass);
  } else {
    WiFi.begin(ssid);  // conexión a red abierta
  }

  for (int i = 0; i < 20; i++) {
    if (WiFi.status() == WL_CONNECTED) {
      wifiConnected = true;
      bleOnly = false;

      // Guardar credenciales
      preferences.begin("wifi", false);
      preferences.putString("ssid", ssid);
      preferences.putString("pass", pass ? pass : "");
      preferences.end();

      if (notifyCallback) notifyCallback("{\"wifiStatus\":\"ok\"}");
      return;
    }
    delay(500);
  }

  wifiConnected = false;
  bleOnly = true;
  if (notifyCallback) notifyCallback("{\"wifiStatus\":\"fail\"}");
}

void WiFiManagerCustom::tryReconnectLastWiFi() {
  preferences.begin("wifi", true);
  String ssid = preferences.getString("ssid", "");
  String pass = preferences.getString("pass", "");
  preferences.end();

  if (ssid.length() > 0) {
    Serial.println("🔁 Intentando reconectar a red guardada...");
    connectToWiFi(ssid.c_str(), pass.length() > 0 ? pass.c_str() : nullptr);
  } else {
    Serial.println("ℹ️ No hay red WiFi guardada.");
  }
}

void WiFiManagerCustom::disconnectWiFi() {
  WiFi.disconnect(true);
  wifiConnected = false;
  bleOnly = true;
  if (notifyCallback) notifyCallback("{\"wifiStatus\":\"disconnected\"}");
}

void WiFiManagerCustom::scanNetworks() {
  scanningNetworks = true;
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  delay(100);

  int n = WiFi.scanNetworks();
  DynamicJsonDocument doc(1024);
  JsonArray arr = doc.to<JsonArray>();

  for (int i = 0; i < n; i++) {
    String s = WiFi.SSID(i);
    if (s.length() == 0) continue;

    JsonObject net = arr.createNestedObject();
    net["ssid"] = s;
    net["open"] = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN);
  }

  String out;
  serializeJson(arr, out);

  const int maxP = 18;
  int cnt = (out.length() + maxP - 1) / maxP;
  for (int i = 0; i < cnt; i++) {
    String frag = "#" + String(i + 1) + out.substring(i * maxP, (i + 1) * maxP);
    if (notifyCallback) notifyCallback(frag);
    delay(100);
  }

  scanningNetworks = false;
}

bool WiFiManagerCustom::isConnected() const { return wifiConnected; }
bool WiFiManagerCustom::isBLEOnly() const { return bleOnly; }
bool WiFiManagerCustom::isScanning() const { return scanningNetworks; }

void WiFiManagerCustom::setNotifyCallback(void (*notify)(const String&)) {
  notifyCallback = notify;
}
