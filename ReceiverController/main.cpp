/*********
  THIROUX Yannis 
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  ©2023 THIROUX Yannis, tous droits réservés.
*********/

/*********PIN
Pin switch : 19 + GND
Pin voltage sensor = 35
Pin Speedometre = 34
Pin Tachymeter = 39
PIn Door Switch = 36

Pin relay IGN3 = 14
Pin relay Accy = 27
Pin relay IGN1 = 26
Pin relay Start = 25
Pin diagMode = 12
PIN Switch LED = 18

Pin relay Unlock = 32
Pin relay Lock = 33


Pont diviseur de tension :
Tachymeter de 12v à 3.36v --> R1 = 10000 Ohm - R2 = 3900 ohm
Speedometer de 8v à 3.31v -->  R1 = 5100 Ohm - R2 = 3600 ohm
*********/
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <BLEAdvertising.h>
#include <Preferences.h>

#include "irk.h"
#include <relay.h>
#include <sensor.h>
#include <OneButton.h>

///////////////////
//Variable Server//
///////////////////

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define AUTOUNLOCK_CHARACTERISTIC_UUID "beb5483f-36e2-4688-b7f5-ea07361b26a8"
#define AUTOLOCKRUN_CHARACTERISTIC_UUID "beb5483a-36e3-4688-b7f5-ea07361b26a8"
#define DIAGMODE_CHARACTERISTIC_UUID "beb5483c-36e5-4688-b7f5-ea07361b26a8"
#define PIN_CHARACTERISTIC_UUID "beb5483d-36e6-4687-b7f5-ea07361b26a8"


#define IRK_LIST_NUMBER 2
const char * IrkListName[IRK_LIST_NUMBER] = {"A","B"};
uint8_t irk[IRK_LIST_NUMBER][ESP_BT_OCTET16_LEN]= {
    // IRK of A
    {0xD2,0x92,0xA0,0x8F,0xF0,0x03,0x81,0x5A,0xD6,0xA2,0xE4,0xE6,0x85,0x57,0xAD,0x35},
    // IRK of B
    {0x85,0x89,0x90,0xBE,0x9C,0xBE,0xBC,0xE7,0xFE,0xEE,0x6B,0xCE,0x5E,0x62,0xE9,0x75}
};

// Declared Pref
Preferences preferences;

//Var to Bluetooth Password
int PASSKEY;

// Button for engine start switch
#define PIN_INPUT 19 // 19 + GND
OneButton button(PIN_INPUT, true);
unsigned long pressStartTime;
unsigned long pressTime;
//Blink Engine Switch led function
bool BlinkSwitchLed = false;


//Var bool on the StartEngine function
bool EngineStarted = false;
bool IgnitionStarted = false;
bool AccyStarted = false;
//Activation Lock Unlock function
bool autoLockUnlock = false;
//Activation Lock Run function
bool autoLockRun = false;
bool StateAutoLockRun = false;
//Diag mode function
bool currentStateDiag = false;


// Range to RSSI
int RSSI_THRESHOLD_OPEN = -70;
int RSSI_THRESHOLD_CLOSED = -90;
//Different Var and bool
bool IphoneDetect = false;
bool proximityDeadZone = false;
bool carOpen = false;
bool deviceConnected = false;

//Val Voltage
float Voltage;


// Service et caractéristique Bluetooth personnalisés
BLEServer* pServer = NULL;
BLECharacteristic* unlockCharacteristic = NULL;
BLECharacteristic* autoCharacteristic = NULL;
BLECharacteristic* autoRunCharacteristic = NULL;
BLECharacteristic* DiagCharacteristic = NULL;
BLECharacteristic* PinCharacteristic = NULL;

///////////////////
//Variable Server//
///////////////////

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
        Serial.println("Authentication Success");
      } else {
        Serial.println("Authentication Failure");
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



////////////////////////////
//DETECT RANDOM MAC IPHONE//
////////////////////////////
void BleDataCheckTask() {
    BLEScan *pBLEScan = BLEDevice::getScan();
    BLEScanResults foundDevices = pBLEScan->start(4, false);  // Scan pendant 3 secondes

    for (int i = 0; i < foundDevices.getCount(); i++) {
        BLEAdvertisedDevice device = foundDevices.getDevice(i);

        BLEAddress AdMac = device.getAddress();
        //printf("Check = %s\r\n", AdMac.toString().c_str());

        for (int j = 0; j < IRK_LIST_NUMBER; j++) {
            if (btm_ble_addr_resolvable((uint8_t *)AdMac.getNative(), irk[j])) {
                //Serial.println("........................................");
                //printf("Mac = %s Belongs to: %s\r\n", AdMac.toString().c_str(), IrkListName[j]);
                Serial.print((String)IrkListName[j] + " is detected,");
                int rssi = device.getRSSI();
                Serial.print(" RSSI: ");
                Serial.print(rssi);
                if (rssi >= RSSI_THRESHOLD_OPEN)
                {
                    Serial.println("--> Device Proximity: Ok");
                    IphoneDetect = true;
                }
                else if (rssi < RSSI_THRESHOLD_OPEN && rssi > RSSI_THRESHOLD_CLOSED)
                {
                    Serial.println("--> Device Proximity:  Dead zone");
                    proximityDeadZone = true;
                }
                else if (rssi <= RSSI_THRESHOLD_CLOSED)
                {
                    Serial.println("--> Device Proximity: Nok");
                }
            }
        }
    }
}


void IphoneDetectFunc(){
  if (autoLockUnlock)
  {
    if (IphoneDetect) {
        if (!carOpen){
            Serial.println("iPhone(s) detected, unlocking the car");
            UnLockRelay();
            delay(2000);
        }
        else {Serial.println("Already open");}
    }
    else if (!IphoneDetect) {
        if (carOpen && !proximityDeadZone) {
            Serial.println("No iphone Detected, locking the car");
            LockRelay();
            delay(2000);
        }
        else {Serial.println("Waiting to Detect Proximity Iphone");}
    }
    
  }
}


// BLE SETVALUE AND NOTIF FUNCTION
void NotifUnlockFunc(int value){
  if (value == 0)
  {
    unlockCharacteristic->setValue("0");
  } else if (value == 1)
  {
    unlockCharacteristic->setValue("1");
  }
  unlockCharacteristic->notify();
}

////////////////////////////
//DETECT RANDOM MAC IPHONE//
////////////////////////////


////////////////////
//BLUETOOTH SERVER//
////////////////////

// Devise Connected Or Disconnected
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
  deviceConnected = true;
  Serial.println("////////////////");
  Serial.println("device connected");
  Serial.println("////////////////");
  };

  void onDisconnect(BLEServer* pServer) {
  deviceConnected = false;
  Serial.println("///////////////////");
  Serial.println("device disconnected");
  Serial.println("///////////////////");
  pServer->getAdvertising()->start();
  }
};


// Write Lock Unlock Characteristics
class UnlockCharacteristicCbs: public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *unlockCharacteristic)
  {
    std::string value = unlockCharacteristic->getValue();

    if (value.length() == 1) {
      int receivedValue = static_cast<int>(value[0]);
      if (receivedValue == 1) {
        UnLockRelay();
      } else if (receivedValue == 0) {
        LockRelay();
      }
    }
  }
};


// Write Activate Lock Unlock Characteristics
class AutoCharacteristicCbs: public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *autoCharacteristic)
  {
    std::string value = autoCharacteristic->getValue();

    if (value.length() == 1) {
      int receivedValue = static_cast<int>(value[0]);
      if (receivedValue == 1) {
        autoCharacteristic->setValue("1");
        autoLockUnlock = true;
        Serial.println("### Auto Lock activate ###");
      } else if (receivedValue == 0) {
        autoCharacteristic->setValue("0");
        autoLockUnlock = false;
        Serial.println("### Auto Lock deactivate ###");
      }
    autoCharacteristic->notify();
    }
  }
};

// Write Activate Auto Lock Run Characteristics
class AutoRunCharacteristicCbs: public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *autoRunCharacteristic)
  {
    std::string value = autoRunCharacteristic->getValue();

    if (value.length() == 1) {
      int receivedValue = static_cast<int>(value[0]);
      if (receivedValue == 1) {
        autoRunCharacteristic->setValue("1");
        autoLockRun = true;
        Serial.println("### Auto Lock  Run activate ###");
      } else if (receivedValue == 0) {
        autoRunCharacteristic->setValue("0");
        autoLockRun = false;
        Serial.println("### Auto Lock Run deactivate ###");
      }
    autoRunCharacteristic->notify();
    }
  }
};

// Write Activate Diag Mode Characteristics
class DiagCharacteristicCbs: public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *DiagCharacteristic)
  {
    std::string value = DiagCharacteristic->getValue();

    if (value.length() == 1) {
      int receivedValue = static_cast<int>(value[0]);
      if (receivedValue == 1) {
        diagMode(HIGH);
        Serial.println("### DIAG MODE ON ###");
      } else if (receivedValue == 0) {
        diagMode(LOW);
        Serial.println("### DIAG MODE OFF ###");
      }
    }
  }
};


// Write to change bluetooth Password Characteristics
class PinCharacteristicCbs: public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *PinCharacteristic)
  {
    std::string value = PinCharacteristic->getValue();

    if (value.length() == 6) {
        PASSKEY = atoi(value.c_str());
        preferences.putUInt("PASSKEY", PASSKEY);
        PinCharacteristic->setValue(PASSKEY);
        Serial.print("PASS KEY CHANGE TO :");
        Serial.println(PASSKEY);
        PinCharacteristic->notify();
        for (int i = 0; i < 10; i++) {  // Faire clignoter 10 fois so 4 secs
          engineSwitchLed(3);
        }
        ESP.restart();
    }
  }
};

////////////////////
//BLUETOOTH SERVER//
////////////////////

//////////
//SWITCH//
//////////
void IRAM_ATTR checkTicks() {
  // include all buttons here to be checked
  // link the xxxclick functions to be called on xxxclick event.
  button.tick(); // just call tick() to check the state.
}

// this function will be called when the button was pressed 1 time only.
void singleClick() {
  Serial.println("singleClick() detected.");
  
  if(!IgnitionStarted && !EngineStarted){
      Serial.println("Ignition Starting...");
      BlinkSwitchLed = true;
      Ignition3(HIGH);
      Accy(HIGH);
      Ignition1(HIGH);
      IgnitionStarted = true;
      AccyStarted = true;
      Serial.println("Ignition Started");
      delay(500);
      return;
      }
  if(IgnitionStarted){
      Ignition3(LOW);
      Accy(LOW);
      Ignition1(LOW);
      EngineStarted = false;
      IgnitionStarted = false;
      AccyStarted = false;
      BlinkSwitchLed = false;
      engineSwitchLed(0);
      if (EngineStarted) {
        Serial.println("Engine Stop");
        if (carOpen == false) {UnLockRelay();}
      }
      else {Serial.println("Ignition/Accy Stop");}
      delay(500);
      return;
      }
} // singleClick


// this function will be called when the button was pressed 2 times in a short timeframe.
void doubleClick() {
  Serial.println("doubleClick() detected.");
  if (!EngineStarted)
  {
    if (!AccyStarted)
    {
      Accy(HIGH);
      BlinkSwitchLed = true;
      AccyStarted = true;
    }
    else if (AccyStarted && !IgnitionStarted)
    {
      Accy(LOW);
      BlinkSwitchLed = false;
      engineSwitchLed(0);
      AccyStarted = false;
    }
  }
} // doubleClick


// this function will be called when the button was pressed multiple times in a short timeframe.
void multiClick() {
  int n = button.getNumberClicks();
  if (n == 3) {
    Serial.println("tripleClick detected.");
  } if (n == 4) {
    Serial.println("quadrupleClick detected.");  

    if (!currentStateDiag && !IgnitionStarted && !AccyStarted && !EngineStarted)
    {
      diagMode(HIGH);
    } 
    else if (!currentStateDiag) {
      diagMode(LOW);
    } 

  } if (n == 5) {
    Serial.println("quintupleClick detected.");
  } else {
    Serial.print("multiClick(");
    Serial.print(n);
    Serial.println(") detected.");
  }
} // multiClick


// this function will be called when the button was held down for 1 second or more.
void pressStart() {
  Serial.println("pressStart()");
  pressStartTime = millis() - 800; // as set in setPressMs()
  if(IgnitionStarted && !EngineStarted){
    BlinkSwitchLed = false;
    Serial.println("Starting Engine...");
    Ignition3(LOW);
    Accy(LOW);
    delay(250);
    Starter(HIGH);
    engineSwitchLed(1);
  }
  
} // pressStart()


// this function will be called when the button was released after a long hold.
void pressStop() {
  pressTime = millis() - pressStartTime;
  Serial.print("pressStop(");
  Serial.print(millis() - pressStartTime);
  Serial.println(") detected.");
  if(IgnitionStarted && !EngineStarted){
    Starter(LOW);
    delay(250);
    Starter(LOW);
    Ignition3(HIGH);
    Accy(HIGH);
    //voltage();
    if (Voltage >= 13.5)
    {
      EngineStarted = true;
      Serial.println("Engine Started");
      return;
    }
    else if (Voltage < 13.5)
    {
      BlinkSwitchLed = true;
      EngineStarted = false;
      Serial.println("Engine Not Started");
      return;
    }
  }
  if (pressTime >= 15000 && !IgnitionStarted && !EngineStarted)
  {
    for (int i = 0; i < 10; i++) {  // Faire clignoter 10 fois so 4 secs
    engineSwitchLed(3);
    }
    ESP.restart();
  }
  
} // pressStop()
//////////
//SWITCH//
//////////


////////////////////
//TASK ON CORE [0]//
////////////////////

//Create Task on the core 0 to update information
TaskHandle_t Task1;
void Task1Scan(void * pvParameters){
      //Serial.print("TaskUpdateInfo running on core ");
      //Serial.println(xPortGetCoreID());
    for(;;){
    voltage();
    if (!IgnitionStarted || !AccyStarted )
    {
      Serial.println("Scan.....");
      IphoneDetect = false; // Réinitialisez la variable avant chaque balayage
      proximityDeadZone = false; // Réinitialisez la variable avant chaque balayage
      BleDataCheckTask();
      IphoneDetectFunc();
    }
    else { 
      Serial.println("STOP SCAN");
      delay(2000);
      }
    if (deviceConnected)
      {
        //voltage();
        DiagCharacteristic->setValue(String(Voltage).c_str());
        Serial.println(Voltage);
        DiagCharacteristic->notify();
      }
    }
  }

////////////////////
//TASK ON CORE [0]//
////////////////////

void setup() {
  Serial.begin(115200);
  //Declare pinMode
  initPin();

  initPosition();

  initpinsensor();
  
  //If Esp retart, check voltage to keep relay ON
  voltage();
  Serial.print("Voltage: ");
  Serial.println(Voltage);
  checkEngineStart();
// Preference
  preferences.begin("KeyLess-Car", false);
  PASSKEY = preferences.getUInt("PASSKEY", 111111);

  // Declare interrupt and switch function
  attachInterrupt(digitalPinToInterrupt(PIN_INPUT), checkTicks, CHANGE);
  button.attachClick(singleClick);
  button.attachDoubleClick(doubleClick);
  button.attachMultiClick(multiClick);
  button.setPressMs(800); // that is the time when LongPressStart is called
  button.attachLongPressStart(pressStart);
  button.attachLongPressStop(pressStop);

  // Initialise le BLE
  BLEDevice::init("KeyLess Car");
  BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);
  BLEDevice::setSecurityCallbacks(new SecurityCallback());
  // Crée le serveur BLE
  pServer = BLEDevice::createServer();
   // Définit la Callback pour les connections
  pServer->setCallbacks(new MyServerCallbacks());
   // Create the BLE Service
  BLEService *pService = pServer->createService(BLEUUID(SERVICE_UUID), 30, 0);

  // Crée le caractéristique BLE
  unlockCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);
  autoCharacteristic = pService->createCharacteristic(AUTOUNLOCK_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);
  autoRunCharacteristic = pService->createCharacteristic(AUTOLOCKRUN_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);
  DiagCharacteristic = pService->createCharacteristic(DIAGMODE_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);
  PinCharacteristic = pService->createCharacteristic(PIN_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);
  // Définit la Callback pour le securite
  unlockCharacteristic->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
  autoCharacteristic->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
  autoRunCharacteristic->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
  DiagCharacteristic->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
  PinCharacteristic->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
  // Définit la Callback pour le caractéristique
  unlockCharacteristic->setCallbacks(new UnlockCharacteristicCbs());
  autoCharacteristic->setCallbacks(new AutoCharacteristicCbs());
  autoRunCharacteristic->setCallbacks(new AutoRunCharacteristicCbs());
  DiagCharacteristic->setCallbacks(new DiagCharacteristicCbs());
  PinCharacteristic->setCallbacks(new PinCharacteristicCbs());
  // Définit la valeur initiale du caractéristique
  unlockCharacteristic->setValue("0");
  autoCharacteristic->setValue("0");
  autoRunCharacteristic->setValue("0");
  DiagCharacteristic->setValue(String(Voltage).c_str());
  //PinCharacteristic->setValue(PASSKEY);
   // Create a BLE Descriptor
  unlockCharacteristic->addDescriptor(new BLE2902());
  autoCharacteristic->addDescriptor(new BLE2902());
  autoRunCharacteristic->addDescriptor(new BLE2902());
  DiagCharacteristic->addDescriptor(new BLE2902());
  PinCharacteristic->addDescriptor(new BLE2902());
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

    //------------------------TASK TO START ENGINE------------------------ 
  xTaskCreatePinnedToCore(Task1Scan, "Task1Scan", 10000, NULL, 1, &Task1, 0);          

}


void loop() {

  button.tick();
  if (BlinkSwitchLed){engineSwitchLed(2);}

// IF ENGINE STOP
  if (!EngineStarted)
  {
    sleepModeFunc();
    AutoShutdownAccyIgn();
    //check if Engine run with key
    //voltage();
    /*if (!IgnitionStarted && carOpen)
    {
      checkEngineStart();
    }*/
    
  }

// IF ENGINE START 
  if (EngineStarted)
  {
    if (carOpen && autoLockRun) {
      AutoLockRun();
    } 
  }

  delay(10);
}