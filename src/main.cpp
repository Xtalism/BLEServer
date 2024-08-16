#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define bleServerName "MANUEL_ESP32"
#define POT_PIN 34  
#define SERVICE_UUID "91bad492-b950-4226-aa2b-4ede9fa42f59"
#define CHARACTERISTIC_UUID "ec0e8b64-33ae-11ec-8d3d-0242ac130003"  // Unique ID for the characteristic

bool deviceConnected = false;
uint32_t pwmValue = 0;

BLECharacteristic *pCharacteristic;

//Setup callbacks onConnect and onDisconnect
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Client Connected");
  };
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Client Disconnected");
  }
};

void setup() {
  Serial.begin(115200);

  // Create the BLE Device
  BLEDevice::init(bleServerName);

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *potService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic for the PWM value
  pCharacteristic = potService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  pCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  potService->start();
  Serial.println("Service started");

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pServer->getAdvertising()->start();
  Serial.println("Waiting for a client connection...");
}

void loop() {
  if (deviceConnected) {
    // Read the potentiometer value
    int potValue = analogRead(POT_PIN);

    // Map the potentiometer value to PWM range (0-255)
    pwmValue = map(potValue, 0, 4095, 0, 255);

    // Send the PWM value to the client
    pCharacteristic->setValue(pwmValue);
    pCharacteristic->notify();

    Serial.print("Sent PWM Value: ");
    Serial.println(pwmValue);

    delay(500);  // Update every 500ms
  } else {
    delay(1000);  // Wait for client connection
  }
}
