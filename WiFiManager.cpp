#include "WiFiManager.h"

void WiFiManagerCustom::begin() {
  WiFi.begin();
}

void WiFiManagerCustom::connectToWiFi(const char* ssid, const char* pass) {
  WiFi.begin(ssid, pass);
  for (int i = 0; i < 20; i++) {
    if (WiFi.status() == WL_CONNECTED) {
      wifiConnected = true;
      bleOnly = false;
      if (notifyCallback) notifyCallback("{\"wifiStatus\":\"ok\"}");
      return;
    }
    delay(500);
  }
  wifiConnected = false;
  bleOnly = true;
  if (notifyCallback) notifyCallback("{\"wifiStatus\":\"fail\"}");
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
    if (s.length() > 0) arr.add(s);
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
