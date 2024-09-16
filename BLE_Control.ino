#include "LineFollower.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define DEVICE_NAME "TM"
#define SERVICE_UUID "3f8a4ae3-cc48-4955-afda-4f2a01d49ec7"
#define KP_UUID "562b2a0b-5752-4f00-8af9-01f7ac0e4bed"
#define KI_UUID "1762ba84-e7c2-4fc8-a70d-8450e6ec010e"
#define KD_UUID "8745bcc3-01a8-4480-ba40-09cbf72bc2f3"
#define CMD_UUID "04eba61a-2f33-45f8-bfce-2843384c56cf"
#define BASESPEED_UUID "39e569ae-a1b0-4f9c-9346-7825cc8eaafd"
#define MAXSPEED_UUID "5f3f3c58-dc9b-4e3a-aa27-4f762e5a14da"

BLEServer* pServer = NULL;

BLECharacteristic* pKp = NULL;
BLECharacteristic* pKi = NULL;
BLECharacteristic* pKd = NULL;
BLECharacteristic* pCmd = NULL;
BLECharacteristic* pBaseSpeed = NULL;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      digitalWrite(LED_BUILTIN,HIGH);
      Serial.println("Client Connected");
    };

    void onDisconnect(BLEServer* pServer) {
      digitalWrite(LED_BUILTIN,LOW);
      Serial.println("Client Disconnected");
      BLEDevice::startAdvertising();
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      String uuid = String(pCharacteristic->getUUID().toString().c_str());
      if (uuid == KP_UUID){
        kp = atof(pCharacteristic->getValue().c_str());
      } else if (uuid == KI_UUID){
        ki = atof(pCharacteristic->getValue().c_str());
      } else if (uuid == KD_UUID){
        kd = atof(pCharacteristic->getValue().c_str());
      } else if (uuid == CMD_UUID){
        int cmd = atoi(pCharacteristic->getValue().c_str());
        switch (cmd) {
          case 0:
            stop();
            Serial.println("Stop");
            break;
          case 1:
            start();
            Serial.println("Start");
            break;
          case 2:
            // write properties
            Serial.println("Save");
            break;
          default:
            Serial.println("Wrong command");
        }
      } else if (uuid == BASESPEED_UUID){
        baseSpeed = atof(pCharacteristic->getValue().c_str());
      }
      Serial.print("Received value ");
      Serial.println(pCharacteristic->getValue().c_str());
    }

    void onRead(BLECharacteristic *pCharacteristic) {
      String uuid = String(pCharacteristic->getUUID().toString().c_str());
      if (uuid == KP_UUID){
        pCharacteristic->setValue(kp);
      } else if (uuid == KI_UUID){
        pCharacteristic->setValue(ki);
      } else if (uuid == KD_UUID){
        pCharacteristic->setValue(kd);
      } else if (uuid == BASESPEED_UUID){
        pCharacteristic->setValue(baseSpeed);
      }
      Serial.println("Sent value");
    }

};

void setupBLE() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // Create the BLE Device
  BLEDevice::init(DEVICE_NAME);

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic *pKp = pService->createCharacteristic(
                                         KP_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  BLECharacteristic *pKi = pService->createCharacteristic(
                                         KI_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  BLECharacteristic *pKd = pService->createCharacteristic(
                                         KD_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  BLECharacteristic *pCmd = pService->createCharacteristic(
                                         CMD_UUID,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  BLECharacteristic *pBaseSpeed = pService->createCharacteristic(
                                         BASESPEED_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pKp->setCallbacks(new MyCallbacks());
  pKp->setValue(kp);

  pKi->setCallbacks(new MyCallbacks());
  pKi->setValue(ki);

  pKd->setCallbacks(new MyCallbacks());
  pKd->setValue(kd);

  pBaseSpeed->setCallbacks(new MyCallbacks());
  pBaseSpeed->setValue(baseSpeed);

  pCmd->setCallbacks(new MyCallbacks());
  
  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pKp->addDescriptor(new BLE2902());
  pKi->addDescriptor(new BLE2902());
  pKd->addDescriptor(new BLE2902());
  pBaseSpeed->addDescriptor(new BLE2902());
  pCmd->addDescriptor(new BLE2902());
  
  // Start the service
  pService->start();

  //BLEAdvertising *pAdvertising = pServer->getAdvertising();
  //pAdvertising->start();
  
  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
}