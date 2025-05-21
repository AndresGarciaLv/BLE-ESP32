#include "CommandParser.h"

void CommandParser::setManagers(WiFiManagerCustom* wifi) {
  wifiManager = wifi;
}

void CommandParser::handleCommand(const String& input) {
  DynamicJsonDocument doc(256);
  DeserializationError error = deserializeJson(doc, input);
  if (error) return;

  if (doc["scan"] == true) {
    wifiManager->scanNetworks();
  } else if (doc["disconnect"] == true) {
    wifiManager->disconnectWiFi();
  } else if (doc.containsKey("ssid") && doc.containsKey("pass")) {
    wifiManager->connectToWiFi(doc["ssid"], doc["pass"]);
  }
}
