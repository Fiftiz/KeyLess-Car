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

// Relai lock et unlock
const int relayUnlock = 32;    //Sur PIN32
const int relayLock = 33;      //Sur PIN33

//Led  Engine Switch
const int switchEngineLed = 19;

void initPin(){  
    pinMode(relayLock, OUTPUT);
    pinMode(relayUnlock, OUTPUT);

    pinMode(relayIGN3, OUTPUT);
    pinMode(relayAccy, OUTPUT);
    pinMode(relayIGN1, OUTPUT);
    pinMode(relayStart, OUTPUT);

    pinMode(switchEngineLed, OUTPUT);
};

void initPosition(){  
    digitalWrite(relayLock, HIGH);
    digitalWrite(relayUnlock, HIGH);

    digitalWrite(switchEngineLed, HIGH);
};
//////////////////////////////////////
////////RELAY LOCK UNLOCK/////////////
//////////////////////////////////////
void UnLockRelay() {
    digitalWrite(relayUnlock, LOW);
    delay(1000);
    digitalWrite(relayUnlock, HIGH);
    delay(500);
    Serial.println("### Unlock ###");

};

void LockRelay() {
    digitalWrite(relayLock, LOW);
    delay(1000);
    digitalWrite(relayLock, HIGH);
    delay(500);
    Serial.println("### Lock ###");

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
void engineSwithLed(int value) {
    if (value == 0) {
    Serial.println("Engine Switch Led : OFF");
    digitalWrite(switchEngineLed, HIGH);
    }
    if (value == 1) {
    Serial.println("Engine Switch Led : ON");
    digitalWrite(switchEngineLed, LOW);
    } 
    if (value == 2) {
    digitalWrite(switchEngineLed, (millis() / 500) % 2);
    }
};

//////////////////////////////////////
////////LED ENGINE SWITCH/////////////
//////////////////////////////////////