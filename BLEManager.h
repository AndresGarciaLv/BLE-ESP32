#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

class CommandParser;

#define SERVICE_UUID        "12345678-1234-1234-1234-1234567890ab"
#define CHARACTERISTIC_UUID "abcd1234-abcd-1234-abcd-1234567890ab"

class BLEManager {
public:
  void begin();
  void sendFragmented(const String& message);
  void notifyWiFiStatus(bool connected, const String& ssid = "");
  void setCommandParser(CommandParser* parser);
  bool isDeviceConnected() const;

private:
  BLECharacteristic* pCharacteristic = nullptr;
  bool deviceConnected = false;
  CommandParser* commandParser = nullptr;
};

#endif
