#include <WiFi.h>
#include <WiFiUdp.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

const char* ssid = "ESP32-Access-Point";
const char* password = "12345678";

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

WiFiUDP udp;
unsigned int localUdpPort = 1234;  // Port utilisé pour les communications UDP
char incomingPacket[255];  // Buffer pour stocker les paquets entrants
BLEServer* pServer = nullptr;
BLECharacteristic* pCharacteristic = nullptr;
bool deviceConnected = false;
bool oldDeviceConnected = false;
const IPAddress multicast_ip(239, 0, 0, 1); 


class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Device connected");
  };

  void onDisconnect(BLEServer* pServer) {
    Serial.println("Device disconnected");
    deviceConnected = false;
    BLEDevice::startAdvertising();
  }
};

class MyCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String value = pCharacteristic->getValue();
    if (value.length() > 0) {
      Serial.println("Received Value: ");
      for (int i = 0; i < value.length(); i++) {
        Serial.print(value[i]);
        udp.beginPacket(IPAddress(239, 0, 0, 1), localUdpPort);
        udp.print(value[i]);
        udp.endPacket();
      }
      Serial.println();
    }
  }
};

void setup() {
  Serial.begin(115200);
  BLEDevice::init("Pulse");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->setCallbacks(new MyCallbacks());

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  BLEDevice::startAdvertising();
  Serial.println("Waiting for a client connection to notify...");

  WiFi.softAP(ssid, password);
  Serial.println(WiFi.softAPIP());

  udp.beginMulticast(IPAddress(239, 0, 0, 1), localUdpPort);

  Serial.printf("Now listening ");
}

void loop() {
  int packetSize = udp.parsePacket();
    if (packetSize) {
      int len = udp.read(incomingPacket, 255);
      if (len > 0) {
      incomingPacket[len] = 0;
      }
      Serial.printf("Message: %s\n", incomingPacket);
      pCharacteristic->setValue(incomingPacket);
      pCharacteristic->notify();
    }
}
