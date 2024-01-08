#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEAdvertising.h>

#include "irk.h"

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define PASSKEY 111111

#define IRK_LIST_NUMBER 2
const char * IrkListName[IRK_LIST_NUMBER] = {"A","B"};
uint8_t irk[IRK_LIST_NUMBER][ESP_BT_OCTET16_LEN]= {
    // IRK of A
    {0xD2,0x92,0xA0,0x8F,0xF0,0x03,0x81,0x5A,0xD6,0xA2,0xE4,0xE6,0x85,0x57,0xAD,0x35},
    // IRK of B
    {0x85,0x89,0x90,0xBE,0x9C,0xBE,0xBC,0xE7,0xFE,0xEE,0x6B,0xCE,0x5E,0x62,0xE9,0x75}
};


// Service et caractéristique Bluetooth personnalisés
BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;

int RSSI_THRESHOLD_OPEN = -50;
int RSSI_THRESHOLD_CLOSED = -70;

bool IphoneDetect = false;
bool proximityDeadZone = false;
bool carOpen = false;

bool deviceConnected = false;

/////////////////////
//BLE Secure Server//
/////////////////////
class SecurityCallback : public BLESecurityCallbacks {

    uint32_t onPassKeyRequest() {
      return 000000;
    }

    void onPassKeyNotify(uint32_t pass_key) {}

    bool onConfirmPIN(uint32_t pass_key) {
      vTaskDelay(5000);
      return true;
    }

    bool onSecurityRequest() {
      return true;
    }

    void onAuthenticationComplete(esp_ble_auth_cmpl_t cmpl) {
      if (cmpl.success) {
        Serial.println("   - SecurityCallback - Authentication Success");
        digitalWrite(LED_BUILTIN, HIGH);
      } else {
        Serial.println("   - SecurityCallback - Authentication Failure*");
        pServer->removePeerDevice(pServer->getConnId(), true);
      }
      BLEDevice::startAdvertising();
    }
};

void bleSecurity() {
  esp_ble_auth_req_t auth_req = ESP_LE_AUTH_REQ_SC_MITM_BOND;
  esp_ble_io_cap_t iocap = ESP_IO_CAP_OUT;
  uint8_t key_size = 16;
  uint8_t init_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
  uint8_t rsp_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
  uint32_t passkey = PASSKEY;
  uint8_t auth_option = ESP_BLE_ONLY_ACCEPT_SPECIFIED_AUTH_DISABLE;
  esp_ble_gap_set_security_param(ESP_BLE_SM_SET_STATIC_PASSKEY, &passkey, sizeof(uint32_t));
  esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &auth_req, sizeof(uint8_t));
  esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(uint8_t));
  esp_ble_gap_set_security_param(ESP_BLE_SM_MAX_KEY_SIZE, &key_size, sizeof(uint8_t));
  esp_ble_gap_set_security_param(ESP_BLE_SM_ONLY_ACCEPT_SPECIFIED_SEC_AUTH, &auth_option, sizeof(uint8_t));
  esp_ble_gap_set_security_param(ESP_BLE_SM_SET_INIT_KEY, &init_key, sizeof(uint8_t));
  esp_ble_gap_set_security_param(ESP_BLE_SM_SET_RSP_KEY, &rsp_key, sizeof(uint8_t));
}
/////////////////////
//BLE Secure Server//
/////////////////////

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


class MyCallbacks: public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    std::string value = pCharacteristic->getValue();

    if (value.length() == 1) {
      if (value[0] == '1') {
        //isOpen = true;
        pCharacteristic->setValue("Open");
        Serial.println("Open");
      } else if (value[0] == '0') {
        //isOpen = false;
        pCharacteristic->setValue("Closed");
        Serial.println("Closed");
      }
    pCharacteristic->notify();
    }
  }
};

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
  deviceConnected = true;
  Serial.println("device connected");
  };

  void onDisconnect(BLEServer* pServer) {
  deviceConnected = false;
  Serial.println("device disconnected");
  pServer->getAdvertising()->start();
  }
};

void setup() {
    Serial.begin(115200);

    // Underclock CPU to Energize save
    setCpuFrequencyMhz(80);
    // Initialise le BLE
    BLEDevice::init("Keyless Car");
    BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);
    BLEDevice::setSecurityCallbacks(new SecurityCallback());
    // Crée le serveur BLE
    pServer = BLEDevice::createServer();
    BLEService *pService = pServer->createService(SERVICE_UUID);
    // Crée le caractéristique BLE
    pCharacteristic = pService->createCharacteristic(
                                        CHARACTERISTIC_UUID,
                                        BLECharacteristic::PROPERTY_READ |
                                        BLECharacteristic::PROPERTY_WRITE |
                                        BLECharacteristic::PROPERTY_NOTIFY |
                                        BLECharacteristic::PROPERTY_INDICATE
                    );
    // Définit la Callback pour le securite
    pCharacteristic->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
    // Définit la Callback pour le caractéristique
    pCharacteristic->setCallbacks(new MyCallbacks());
    // Définit la Callback pour les connections
    pServer->setCallbacks(new MyServerCallbacks());
    // Définit la valeur initiale du caractéristique
    pCharacteristic->setValue("Closed");
    // Démarre le service BLE
    pService->start();
    // Commence à diffuser le service BLE
    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    pAdvertising->start();

    bleSecurity();
    Serial.println("Bluetooth Server ok");
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