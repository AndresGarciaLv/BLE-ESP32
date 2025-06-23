#include "BLEManager.h"
#include "CommandParser.h"
#include "mbedtls/base64.h"
#include <ArduinoJson.h>

class CharacteristicCallbacks : public BLECharacteristicCallbacks {
public:
  CharacteristicCallbacks(CommandParser* parser) : parser(parser) {}

  String decodeBase64(const String& encoded) {
    size_t outLen;
    std::string input = encoded.c_str();
    size_t inputLen = input.length();

    size_t maxOutputLen = (inputLen * 3) / 4 + 1;
    unsigned char* output = (unsigned char*)malloc(maxOutputLen);
    if (!output) return "";

    int ret = mbedtls_base64_decode(output, maxOutputLen, &outLen, (const unsigned char*)input.c_str(), inputLen);
    if (ret != 0) {
      free(output);
      return "";
    }

    String decoded = String((char*)output);
    free(output);
    return decoded;
  }

  void onWrite(BLECharacteristic* c) override {
    Serial.println("🔡 BLE onWrite triggered");

    String raw = c->getValue();
    Serial.print("📉 Mensaje recibido por BLE: ");
    Serial.println(raw);

    String decoded = decodeBase64(raw);
    String incoming = decoded.length() > 0 ? decoded : raw;

    Serial.print("📨 Decodificado JSON: ");
    Serial.println(incoming);

    if (!parser) {
      Serial.println("❌ Parser no inicializado.");
      return;
    }

    parser->handleCommand(incoming);
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

  Serial.println("🔧 BLE inicializado y en espera de conexión.");
}

void BLEManager::sendFragmented(const String& message) {
  if (!deviceConnected || !pCharacteristic) return;

  const int maxLength = 16;
  int totalLength = message.length();
  int parts = (totalLength + maxLength - 1) / maxLength;

  for (int i = 0; i < parts; i++) {
    String content = message.substring(i * maxLength, (i + 1) * maxLength);
    String fragment = "#" + String(i + 1) + "|" + content;

    Serial.print("📤 Enviando fragmento: ");
    Serial.println(fragment);

    pCharacteristic->setValue(fragment.c_str());
    pCharacteristic->notify();
    delay(80);
  }

  Serial.println("📤 Enviando fragmento final: #END");
  pCharacteristic->setValue("#END");
  pCharacteristic->notify();
}

void BLEManager::notifyWiFiStatus(bool connected, const String& ssid) {
  if (!deviceConnected || !pCharacteristic) return;

  StaticJsonDocument<128> doc;
  JsonObject status = doc.createNestedObject("wifiStatus");
  status["connected"] = connected;
  if (connected) {
    doc["ssid"] = ssid;
  }

  String json;
  serializeJson(doc, json);

  Serial.print("📶 Notificando estado WiFi: ");
  Serial.println(json);

  pCharacteristic->setValue(json.c_str());
  pCharacteristic->notify();
}

void BLEManager::setCommandParser(CommandParser* parser) {
  this->commandParser = parser;
}

bool BLEManager::isDeviceConnected() const {
  return deviceConnected;
}
