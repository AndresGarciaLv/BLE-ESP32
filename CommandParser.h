#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#include <ArduinoJson.h>
#include "WiFiManager.h"

class CommandParser {
public:
  void setManagers(WiFiManagerCustom* wifi);
  void handleCommand(const String& input);

private:
  WiFiManagerCustom* wifiManager = nullptr;
};

#endif
