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
  ServerCallbacks(bool* deviceConnected) : deviceConnected(deviceConnected) {}

  void onConnect(BLEServer*) override {
    *deviceConnected = true;
  }

  void onDisconnect(BLEServer*) override {
    *deviceConnected = false;
    BLEDevice::startAdvertising();
  }

private:
  bool* deviceConnected;
};

void BLEManager::begin() {
  BLEDevice::init("PelucheBLE");
  BLEServer* server = BLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks(&deviceConnected));

  BLEService* service = server->createService(SERVICE_UUID);
  pCharacteristic = service->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_WRITE
  );

  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->setCallbacks(new CharacteristicCallbacks(commandParser));
  service->start();
  BLEDevice::startAdvertising();
}

void BLEManager::sendFragmented(const String& message) {
  const int maxPayloadSize = 17; // 17 + 1 (por índice "#x|") = 20 máx para BLE
  int index = 1;
  int start = 0;
  int len = message.length();

  Serial.println("========== JSON COMPLETO ==========");
  Serial.println(message);
  Serial.println("====================================");
  Serial.println("== FRAGMENTANDO JSON PARA BLE =====");

  while (start < len) {
    int end = start + maxPayloadSize;
    if (end > len) end = len;

    // ⚠️ Aseguramos no cortar dentro de una palabra
    while (end < len && message.charAt(end) != ',' && message.charAt(end) != '}') {
      end--;
      if (end <= start) break; // safety fallback
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
