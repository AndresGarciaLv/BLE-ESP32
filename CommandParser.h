#ifndef COMMANDPARSER_H
#define COMMANDPARSER_H

#include <Arduino.h>

class WiFiManagerCustom;

class CommandParser {
public:
  void setManagers(WiFiManagerCustom* wifi);
  void handleCommand(const String& input);

private:
  WiFiManagerCustom* wifiManager = nullptr;
};

#endif
