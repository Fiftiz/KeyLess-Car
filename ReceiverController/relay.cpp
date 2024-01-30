/*********
  THIROUX Yannis 
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  ©2023 THIROUX Yannis, tous droits réservés.
*********/

#include <Arduino.h>
#include <relay.h>

const int relayIGN3 = 14;        //Sur PIN14
const int relayAccy = 27;        //Sur PIN27
const int relayIGN1 = 26;        //Sur PIN26
const int relayStart = 25;     //Sur PIN25

// Diag Mode
const int diagModePin = 12;
extern bool currentStateDiag;

//Led  Engine Switch
const int switchEngineLed = 18;
int brightness = 10;    // how bright the LED is
int fadeAmount = 5;    // how many points to fade the LED by

// Relai lock et unlock
const int relayUnlock = 32;    //Sur PIN32
const int relayLock = 33;      //Sur PIN33
extern bool carOpen;
//Auto lock run
extern bool StateAutoLockRun;



void initPin(){  
    pinMode(relayLock, OUTPUT);
    pinMode(relayUnlock, OUTPUT);

    pinMode(relayIGN3, OUTPUT);
    pinMode(relayAccy, OUTPUT);
    pinMode(relayIGN1, OUTPUT);
    pinMode(relayStart, OUTPUT);
    // pinmode if analog read switchEngineLed
    pinMode(switchEngineLed, OUTPUT);

    pinMode(diagModePin, OUTPUT);
};

void initPosition(){  
    digitalWrite(relayLock, HIGH);
    digitalWrite(relayUnlock, HIGH);
};
//////////////////////////////////////
////////RELAY LOCK UNLOCK/////////////
//////////////////////////////////////
void UnLockRelay() {
    digitalWrite(relayUnlock, LOW);
    delay(1000);
    digitalWrite(relayUnlock, HIGH);
    carOpen = true;
    NotifUnlockFunc(1); // Fonction in main.cpp to change setvalue and noti bluetooth
    StateAutoLockRun = false;
    Serial.println("### Unlock ###");
    delay(500);
};

void LockRelay() {
    digitalWrite(relayLock, LOW);
    delay(1000);
    digitalWrite(relayLock, HIGH);
    carOpen = false;
    NotifUnlockFunc(0); // Fonction in main.cpp to change setvalue and noti bluetooth
    Serial.println("### Lock ###");
    delay(500);
};
////////////////////////////////////////////
///////////IGN + ACCY + STARTER/////////////
////////////////////////////////////////////
void Ignition3(uint8_t value) {
  digitalWrite(relayIGN3, value);
  if (value == HIGH) {
    Serial.println("IGN3: ON");
  } else {Serial.println("IGN3: OFF");}
};

void Accy(uint8_t value) {
  digitalWrite(relayAccy, value);
  if (value == HIGH) {
    Serial.println("ACCY: ON");
  } else {Serial.println("ACCY: OFF");}
};

void Ignition1(uint8_t value) {
  digitalWrite(relayIGN1, value);
  if (value == HIGH) {
    Serial.println("IGN1: ON");
  } else {Serial.println("IGN1: OFF");}
};

void Starter(uint8_t value) {
  digitalWrite(relayStart, value);
  if (value == HIGH) {
    Serial.println("STARTER : ON");
  } else {Serial.println("STARTER: OFF");}
};
//////////////////////////////////////
////////LED ENGINE SWITCH/////////////
//////////////////////////////////////
void engineSwitchLed(int value) {
    if (value == 0) {
      Serial.println("Engine Switch Led : OFF");
      analogWrite(switchEngineLed, 0);
    }
    if (value == 1) {
      Serial.println("Engine Switch Led : ON");
      analogWrite(switchEngineLed, 255);
    } 
    if (value == 2) {
      const unsigned long adjustInterval = 10;
      static unsigned long lastAdjust = millis();
      if ( millis() - lastAdjust > adjustInterval) {
        lastAdjust += adjustInterval;
        // set the brightness of pin 9:
        analogWrite(switchEngineLed, brightness);

        // change the brightness for next time through the loop:
        brightness = brightness + fadeAmount;

        // reverse the direction of the fading at the ends of the fade:
        if (brightness <= 10 || brightness >= 255) {
          fadeAmount = -fadeAmount;
        }
        // wait for 30 milliseconds to see the dimming effect
      } // end of millis() adjustInterval code
    }
    if (value == 3) {
      analogWrite(switchEngineLed, 255);
      delay(200);
      analogWrite(switchEngineLed, 0);
      delay(200);
    }
    //digitalWrite(switchEngineLed, (millis() / 500) % 2);
    //analogWrite(switchEngineLed, (millis() / 1000) % 2 == 0 ? 255 : 0);
};
////////////////////////////////////
////////DIAGNOSTOC MODE/////////////
////////////////////////////////////
void diagMode(uint8_t value) {
  if (value == HIGH){
    digitalWrite(diagModePin, HIGH);
    currentStateDiag = true;
  } else {
    digitalWrite(diagModePin, LOW);
    currentStateDiag = false;
  }
};