#include "WebSocketManager.h"

void WebSocketManager::connect(const char* url) {
  if (!connected) {
    connected = client.connect(url);
    if (connected) {
      Serial.println("✅ WebSocket conectado correctamente a:");
      Serial.println(url);

     // Asignar el callback al cliente WebSocket
      client.onMessage([this](WebsocketsMessage message) {
        if (messageCallback) {
          messageCallback(message.data());
        }
      });

    } else {
      Serial.println("❌ Error al conectar WebSocket.");
    }
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

void WebSocketManager::setOnMessageCallback(std::function<void(const String&)> cb) {
  messageCallback = cb;
}