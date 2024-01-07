#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEAdvertising.h>

#include "irk.h"

#define IRK_LIST_NUMBER 2
const char * IrkListName[IRK_LIST_NUMBER] = {"A","B"};
uint8_t irk[IRK_LIST_NUMBER][ESP_BT_OCTET16_LEN]= {
    // IRK of A
    {0xD2,0x92,0xA0,0x8F,0xF0,0x03,0x81,0x5A,0xD6,0xA2,0xE4,0xE6,0x85,0x57,0xAD,0x35},
    // IRK of B
    {0x85,0x89,0x90,0xBE,0x9C,0xBE,0xBC,0xE7,0xFE,0xEE,0x6B,0xCE,0x5E,0x62,0xE9,0x75}
};
 

BLEServer* pServer;

int RSSI_THRESHOLD_OPEN = -50;
int RSSI_THRESHOLD_CLOSED = -70;

bool IphoneDetect = false;
bool proximityDeadZone = false;
bool carOpen = false;

void BleDataCheckTask() {
    BLEScan *pBLEScan = BLEDevice::getScan();
    BLEScanResults foundDevices = pBLEScan->start(3, false);  // Scan pendant 3 secondes

    bool proximityOk = false;
    bool proximityNok = false;

    for (int i = 0; i < foundDevices.getCount(); i++) {
        BLEAdvertisedDevice device = foundDevices.getDevice(i);

        BLEAddress AdMac = device.getAddress();
        //printf("Check = %s\r\n", AdMac.toString().c_str());


        for (int j = 0; j < IRK_LIST_NUMBER; j++) {
            if (btm_ble_addr_resolvable((uint8_t *)AdMac.getNative(), irk[j])) {
                //Serial.println("........................................");
                printf("Mac = %s Belongs to: %s\r\n", AdMac.toString().c_str(), IrkListName[j]);

                int rssi = device.getRSSI();
                Serial.print("RSSI: ");
                Serial.print(rssi);
                if (rssi >= RSSI_THRESHOLD_OPEN)
                {
                    Serial.println(" Device Proximity : Ok");
                    Serial.println(" ");
                    //Serial.println("........................................");
                    proximityOk = true;
                }
                if (rssi < RSSI_THRESHOLD_OPEN && rssi > RSSI_THRESHOLD_CLOSED)
                {
                    Serial.println(" Device Proximity :  Dead zone");
                    Serial.println(" ");
                    //Serial.println("........................................");
                    proximityDeadZone = true;
                }
                if (rssi <= RSSI_THRESHOLD_CLOSED)
                {
                    Serial.println(" Device Proximity : Nok");
                    Serial.println(" ");
                    //Serial.println("........................................");
                    proximityNok = true;
                }
            }
        }
    }
    if (proximityOk) {
        IphoneDetect = true;
    }
    if (proximityNok && !proximityOk && !proximityDeadZone) {
        IphoneDetect = false;
    }
}


//class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
//{
//    void onResult(BLEAdvertisedDevice advertisedDevice)
//    {
//        // Callback when a BLE device is found
//    }
//};


void setup() {
    Serial.begin(115200);

    // Underclock CPU to Energize save
    setCpuFrequencyMhz(80);

    BLEDevice::init("Keyless Car");
    BLEServer *pServer = BLEDevice::createServer();

    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(BLEUUID("00001800-0000-1000-8000-00805F9B34FB"));
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);

    pServer->getAdvertising()->start();
}

void loop() {
    Serial.println("Scan.....");
    IphoneDetect = false; // Réinitialisez la variable avant chaque balayage
    proximityDeadZone = false;
    BleDataCheckTask();

    if (!IphoneDetect) {
        if (!carOpen) {
            Serial.println("Waiting to Detect Iphone");
        }
        if (carOpen && !proximityDeadZone) {
            Serial.println("No iphone Detected, Closing the car");
            carOpen = false;
            delay(5000);
        }
    }
    if (IphoneDetect) {
        if (carOpen) {
            Serial.println("Awaiting... Car is open");
        }
        if (!carOpen){
            Serial.println("iPhone(s) detected, opening the car");
            carOpen = true;
            delay(5000);
        }
    }
    Serial.println("-------------------------------------");

}