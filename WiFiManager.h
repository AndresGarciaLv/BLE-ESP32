#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include <ArduinoJson.h>

class WiFiManagerCustom {
public:
  void begin();
  void connectToWiFi(const char* ssid, const char* pass);
  void disconnectWiFi();
  void scanNetworks();
  bool isConnected() const;
  bool isBLEOnly() const;
  bool isScanning() const;

  void setNotifyCallback(void (*notify)(const String&));

private:
  bool wifiConnected = false;
  bool bleOnly = true;
  bool scanningNetworks = false;
  void (*notifyCallback)(const String&) = nullptr;
};

#endif
