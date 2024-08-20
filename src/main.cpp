#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define bleServerName "NAME"
#define POT_PIN 2
#define SERVICE_UUID "91bad492-b950-4226-aa2b-4ede9fa42f59"
#define CHARACTERISTIC_UUID "ec0e8b64-33ae-11ec-8d3d-0242ac130003"  // Unique ID for the characteristic

bool deviceConnected = false;
BLEServer *pServer = nullptr;
uint32_t pwmValue = 0;

BLECharacteristic *pCharacteristic;

// MAC address 
const char* allowedClientMAC = "XX:XX:XX:XX:XX:XX";

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Client connected");
  };

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Client disconnected");
  }
};

class MyCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    Serial.print("Received value: ");
    Serial.println(value.c_str());

    // Check if the received value is the allowed MAC address
    if (value == allowedClientMAC) {
      Serial.println("Allowed client connected");
    } else {
      Serial.println("Unauthorized client tried to connect");
      pServer->disconnect(0);
    }
  }
};

void setup() {
  Serial.begin(115200);

  BLEDevice::init(bleServerName);

  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *potService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic for the PWM value
  pCharacteristic = potService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_WRITE
                    );

  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->setCallbacks(new MyCallbacks());

  potService->start();
  Serial.println("Service started");

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pServer->getAdvertising()->start();
  Serial.println("Waiting for a client connection...");
}

void loop() {
  if (deviceConnected) {
    int potValue = analogRead(POT_PIN);

    pwmValue = map(potValue, 0, 4095, 0, 255);

    pCharacteristic->setValue(pwmValue);
    pCharacteristic->notify();

    Serial.print("Sent PWM Value: ");
    Serial.println(pwmValue);

    delay(500);
  } else {
    delay(1000);  // Wait for client connection
  }
}