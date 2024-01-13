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
#include <sensor.h>

//Relai 30A de demarrage
const int startSwitchPin = 21; //BOUTON SUR PIN36, Autre fil sur GND en pullup
int startSwitchState = 0;
const int relayIGN3 = 14;        //Sur PIN14
const int relayAccy = 27;        //Sur PIN27
const int relayIGN1 = 26;        //Sur PIN26
const int relayStart = 25;     //Sur PIN25

extern bool EngineStarted;
extern bool flowEngine;
extern bool IgnitionStarted;

// Relai lock et unlock
const int relayUnlock = 32;    //Sur PIN32
const int relayLock = 33;      //Sur PIN 27

extern float Voltage;


#define DEBOUNCE_TIME  50 // the debounce time in millisecond, increase this time if it still chatters
// Variables will change:
int lastSteadyState = LOW;       // the previous steady state from the input pin
int lastFlickerableState = LOW;  // the previous flickerable state from the input pin
int currentState;
// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled




void initPin(){  
    pinMode(relayLock, OUTPUT);
    pinMode(relayUnlock, OUTPUT);

    pinMode(relayIGN3, OUTPUT);
    pinMode(relayAccy, OUTPUT);
    pinMode(relayIGN1, OUTPUT);
    pinMode(relayStart, OUTPUT);

    pinMode(startSwitchPin, INPUT_PULLUP);
};

void initPosition(){  
    digitalWrite(relayLock, HIGH);
    digitalWrite(relayUnlock, HIGH);

    digitalWrite(relayIGN3, HIGH);
    digitalWrite(relayAccy, HIGH);
    digitalWrite(relayIGN1, HIGH);
    digitalWrite(relayStart, HIGH);
};

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
//////////////////////////////////////
void IgnitionOFFduringStart(uint8_t value) {
  digitalWrite(relayIGN3, value);
  digitalWrite(relayAccy, value);
  if (value == LOW) {
    Serial.println("IGN3 and ACCY: ON");
  } else {Serial.println("IGN3 and ACCY: OFF");}
};

void IgnitionONduringStart(uint8_t value) {
  digitalWrite(relayIGN1, value);
  if (value == LOW) {
    Serial.println("IGN1: ON");
  } else {Serial.println("IGN1: OFF");}
};



void StartEngine() {
   // read the state of the switch/button:
  currentState = digitalRead(startSwitchPin);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch/button changed, due to noise or pressing:
  if (currentState != lastFlickerableState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
    // save the the last flickerable state
    lastFlickerableState = currentState;
  }

  if ((millis() - lastDebounceTime) > DEBOUNCE_TIME) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if(lastSteadyState == HIGH && currentState == LOW && IgnitionStarted && !EngineStarted){
      Serial.println("Starting Engine...");
      IgnitionOFFduringStart(HIGH);
      delay(500);
      digitalWrite(relayStart, LOW);
      }
    else if(lastSteadyState == LOW && currentState == HIGH && IgnitionStarted && !EngineStarted){
      digitalWrite(relayStart, HIGH);
      delay(500);
      IgnitionOFFduringStart(LOW);
      //EngineStarted = true;
      //Serial.println("Engine Started");
      int i= 0;
      while (i <= 400)
        {
        voltage();
        i++;
        delay(5);
        }
        if (Voltage >= 13)
        {
          EngineStarted = true;
          Serial.println("Engine Started");
        }
        else {
          EngineStarted = false;
          Serial.println("Engine Not Started");
        }
      }


    else if(lastSteadyState == HIGH && currentState == LOW && EngineStarted){
      IgnitionOFFduringStart(HIGH);
      IgnitionONduringStart(HIGH);
      EngineStarted = false;
      IgnitionStarted = false;
      Serial.println("Engine Stop");
      }


    else if(lastSteadyState == HIGH && currentState == LOW &&!IgnitionStarted && !EngineStarted){
      Serial.println("Ignition Starting...");
      IgnitionOFFduringStart(LOW);
      IgnitionONduringStart(LOW);
      flowEngine = true;
      }
    else if(lastSteadyState == LOW && currentState == HIGH &&!IgnitionStarted && !EngineStarted &&flowEngine){
      IgnitionStarted = true;
      flowEngine = false;
      Serial.println("Ignition Started");
      }


    // save the the last steady state
    lastSteadyState = currentState;
  }
};