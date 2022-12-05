// Author: Joel Nail
// NCIR code modified from https://github.com/m5stack/M5StickC/blob/master/examples/Hat/NCIR_HAT/NCIR_HAT.ino
// Inspiration also taken from https://randomnerdtutorials.com/esp32-ble-server-client/ 

//#include <M5StickC.h>
#include "M5StickCPlus.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <Wire.h>

#include <string>
using namespace std;


// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "a18e2b83-e373-4bc3-9a05-65993dd6c638"
#define CHARACTERISTIC_UUID "23b72014-de58-4bea-b898-aef52ed79a79"
#define DANGER_CHARACTERISTIC_UUID "4100e5d9-4bde-4b70-ab0f-6c8c309d726c"

BLECharacteristic tempCharacteristic("4100e5d9-4bde-4b70-ab0f-6c8c309d726c", BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);



void setup() {
    M5.begin();
    Wire.begin(0, 26);
    Serial.begin(115200);
    //M5.Lcd.setRotation(3);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(3);
    // M5.Lcd.clear(BLACK);
    M5.Lcd.setCursor(60, 160);
    M5.Beep.setBeep(100, 500);
    M5.Beep.setVolume(2);

    BLEDevice::init("TEMPAWTURE_DANGER_ALERT");
    // Creating BLE server
    BLEServer *pServer = BLEDevice::createServer();
    // Creating the service and characteristic using the UUIDs defined above
    BLEService *pService = pServer->createService(SERVICE_UUID);
    pService->addCharacteristic(&tempCharacteristic);
    
    // Setting initial value for the characteristic
    tempCharacteristic.setValue("Initial Connection");
    // Launching server
    pService->start();

    // Beginning advertisement of packages
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
}

void advBLE(float temp) {
    // Defining the message to be sent via Bluetooth
    string alertBegin = "YOUR PET IS AT A DANGEROUS TEMPERATURE. MEASURED TEMPERATURE: ";
    string alertEnd = " DEGREES FARENHEIT";
    string toSend = alertBegin + std::to_string(temp) + alertEnd;

    tempCharacteristic.setValue(toSend);
    
    // Setting initial value for the characteristic
    // pCharacteristic->setValue(toSend);
}

uint16_t result;
float temperature;
float temperatureF;
float fConst;

void loop() {
    Wire.beginTransmission(0x5A);  // Send Initial Signal and I2C Bus Address
    Wire.write(0x07);  // Send data only once and add one address automatically.
    Wire.endTransmission(false);  // Stop signal
    Wire.requestFrom(
        0x5A,
        2);  // Get 2 consecutive data from 0x5A, and the data is stored only.
    result = Wire.read();        // Receive DATA
    result |= Wire.read() << 8;  // Receive DATA

    temperature = result * 0.02 - 273.15;
    fConst = 32;
    temperatureF = fConst + (temperature * 1.8);

    M5.Lcd.fillRect(0, 40, 120, 100, BLACK);
    M5.Lcd.setCursor(25, 100);

    M5.Lcd.print(temperatureF);
    Serial.println(temperatureF);

    // if a dangerous temperature is detected, advertise an alert to the user
    if (temperatureF > 90) {
      advBLE(temperatureF);

      //tempCharacteristic.setValue(temperatureF);
      
      M5.Beep.update();
      M5.Beep.beep();

      delay(10000);
    }




    delay(5000);
    M5.update();

}