#ifndef WEBSOCKET_MANAGER_H
#define WEBSOCKET_MANAGER_H

#include <ArduinoWebsockets.h>
#include <functional>

using namespace websockets;

class WebSocketManager {
public:
  void connect(const char* url);
  void send(const String& message);
  void poll();
  bool isAvailable() const;

  // Nuevo: asignar callback para recibir mensajes
  void setOnMessageCallback(std::function<void(const String&)> cb);

private:
  WebsocketsClient client;
  bool connected = false;
  std::function<void(const String&)> messageCallback;
};

#endif
