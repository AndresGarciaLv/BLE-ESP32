#ifndef WEBSOCKET_MANAGER_H
#define WEBSOCKET_MANAGER_H

#include <ArduinoWebsockets.h>

using namespace websockets;

class WebSocketManager {
public:
  void connect(const char* url);
  void send(const String& message);
  void poll();
  bool isAvailable() const;

private:
  WebsocketsClient client;
  bool connected = false;
};

#endif
