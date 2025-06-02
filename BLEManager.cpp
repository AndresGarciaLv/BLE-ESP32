#include "BLEManager.h"

class CharacteristicCallbacks : public BLECharacteristicCallbacks {
public:
  CharacteristicCallbacks(CommandParser* parser) : parser(parser) {}

  void onWrite(BLECharacteristic* c) override {
    if (parser) {
      parser->handleCommand(c->getValue().c_str());
    }
  }

private:
  CommandParser* parser;
};

class ServerCallbacks : public BLEServerCallbacks {
public:
  ServerCallbacks(bool* deviceConnected, BLECharacteristic* characteristic)
    : deviceConnected(deviceConnected), pCharacteristic(characteristic) {}

  void onConnect(BLEServer*) override {
    *deviceConnected = true;
    Serial.println("✅ BLE conectado: dispositivo enlazado :).");
  }

  void onDisconnect(BLEServer*) override {
    *deviceConnected = false;
    Serial.println("❌ BLE desconectado: esperando nuevo dispositivo...");

    // Enviar estado BLE a la app si aún está permitido
    if (pCharacteristic) {
      String msg = "{\"bleStatus\":\"disconnected\"}";
      pCharacteristic->setValue(msg.c_str());
      pCharacteristic->notify();
      delay(100);
    }

    BLEDevice::startAdvertising();
  }

private:
  bool* deviceConnected;
  BLECharacteristic* pCharacteristic;
};

void BLEManager::begin() {
  BLEDevice::init("Peluche BLE");
  BLEServer* server = BLEDevice::createServer();

  BLEService* service = server->createService(SERVICE_UUID);
  pCharacteristic = service->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_WRITE
  );

  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->setCallbacks(new CharacteristicCallbacks(commandParser));

  server->setCallbacks(new ServerCallbacks(&deviceConnected, pCharacteristic));

  service->start();
  BLEDevice::startAdvertising();
}

void BLEManager::sendFragmented(const String& message) {
  if (!deviceConnected) {
    Serial.println("⚠️ BLE desconectado, no se enviará fragmento.");
    return;
  }

  const int maxPayloadSize = 17;
  int index = 1;
  int start = 0;
  int len = message.length();

  Serial.println("========== JSON COMPLETO ==========");
  Serial.println(message);
  Serial.println("====================================");
  Serial.println("== FRAGMENTANDO JSON PARA BLE =====");

  while (start < len) {
    // 🛑 Detener envío si se desconecta en medio del fragmentado
    if (!deviceConnected) {
      Serial.println("⛔ BLE se desconectó a mitad del envío. Abortando fragmentación.");
      break;
    }

    int end = start + maxPayloadSize;
    if (end > len) end = len;

    while (end < len && message.charAt(end) != ',' && message.charAt(end) != '}') {
      end--;
      if (end <= start) break;
    }

    String payload = message.substring(start, end);
    String fragment = "#" + String(index) + "|" + payload;

    Serial.print("FRAGMENTO #");
    Serial.print(index);
    Serial.print(": ");
    Serial.println(fragment);

    pCharacteristic->setValue(fragment.c_str());
    pCharacteristic->notify();
    delay(100);

    start = end;
    index++;
  }

  Serial.println("========= FRAGMENTACIÓN LISTA =======");
}


void BLEManager::setCommandParser(CommandParser* parser) {
  this->commandParser = parser;
}

bool BLEManager::isDeviceConnected() const {
  return deviceConnected;
}
