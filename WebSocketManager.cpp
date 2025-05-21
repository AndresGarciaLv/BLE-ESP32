#include "WebSocketManager.h"

void WebSocketManager::connect(const char* url) {
  if (!connected) {
    connected = client.connect(url);
  }
}

void WebSocketManager::send(const String& message) {
  if (connected) {
    client.send(message);
  }
}

void WebSocketManager::poll() {
  if (connected) {
    client.poll();
  }
}

bool WebSocketManager::isAvailable() const {
  return connected;
}
