#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <functional>

class BLEManager;

class WiFiManagerCustom {
public:
  void begin();
  void connectToWiFi(const char* ssid, const char* pass);
  void tryReconnectLastWiFi();
  void disconnectFromWiFi();
  void scanNetworks();
  bool isConnected() const;
  bool isScanning() const;
  void notifyWiFiStatus();


  void setBLEManager(BLEManager* ble);
  void setNotifyCallback(std::function<void(const String&)> callback);
  void setSendDataControl(std::function<void(bool)> control);

private:
  BLEManager* bleManager = nullptr;
  std::function<void(const String&)> notifyCallback = nullptr;
  std::function<void(bool)> sendDataControl = nullptr;
  bool wifiConnected = false;
  bool scanning = false;
  bool bleOnly = true;
};

#endif
