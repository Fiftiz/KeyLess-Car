/*********
  THIROUX Yannis 
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  ©2023 THIROUX Yannis, tous droits réservés.
*********/

/*Pont diviseur de tension :

de 12v à 3.36v --> R1 = 10000 Ohm - R2 = 3900 ohm
de 8v à 3.31v -->  R1 = 5100 Ohm - R2 = 3600 ohm

*/

#include <Arduino.h>
#include <sensor.h>
#include <relay.h>

extern bool EngineStarted;
extern bool IgnitionStarted;
extern bool AccyStarted;

extern bool BlinkSwitchLed;

extern bool carOpen;

//PIN INPUT Voltage sensor
const int voltageSensor = 35;
float VoltT;
extern float Voltage;
float Offset = -0.18;

//Input speedometre
const int Speedometre = 34;
float VoltSpeedo;
float Speedo;
extern bool StateAutoLockRun;
//Input tachymeter
const int tachymetre = 39;
float VoltTachy;
float Rpm;

//Input tachymeter
const int doorSwitchPin = 36;
float VoltDoorSwitch;
float doorSwitch;

//Timer Auto Shutdown
unsigned long AS_previousMillis = 0;
bool AS_StartTimer = false;

// SleepMode
bool SleepModeReady = false;
unsigned long SM_previousMillis = 0;
bool SM_startTimer = false;


void initpinsensor(){ 
  pinMode(voltageSensor, INPUT);
  pinMode(Speedometre, INPUT);
  pinMode(tachymetre, INPUT);
  pinMode (doorSwitchPin, INPUT);
};
/////////////////////////////////
////////VOLTAGE CALCULATE////////
/////////////////////////////////

// ***** function calcul la moyen de la valeur de l'analogRead******
float getVPP()
{
  float result;
  int readValue;                // value read from the sensor
  int maxValue = 0;             // store max value here
  int minValue = 4096;          // store min value here ESP32 ADC resolution
  
   uint32_t start_time = millis();
   while((millis()-start_time) < 1000) //sample for 2 Sec
   {
       readValue = analogRead(voltageSensor);
       // see if you have a new maxValue
       if (readValue > maxValue) 
       {
           /*record the maximum sensor value*/
           maxValue = readValue;
       }
       if (readValue < minValue) 
       {
           /*record the minimum sensor value*/
           minValue = readValue;
       }
   }
   // Subtract min from max
   result = ((maxValue + minValue) / 2) * 3.3/4096.0; //ESP32 ADC resolution 4096
   return result;
 }

void voltage(){ 
  VoltT = getVPP();
  //Serial.println(VoltT);
  Voltage = (VoltT - Offset) * 16.5 / 3.3; //Calculated Voltage in V
};
//////////////////////////////////
////////CHECK ENGINE START////////
//////////////////////////////////
void checkEngineStart(){ 
  if (Voltage >= 13.5)
  {
    EngineStarted = true;
    IgnitionStarted = true;
    Ignition3(HIGH);
    Accy(HIGH);
    Ignition1(HIGH);
    engineSwitchLed(1);

    Serial.println("Engine is started.");
  }
  else {
    EngineStarted = false;
    Serial.println("Engine isn't started.");
  }
};
/////////////////////////////////
////////SPEEDO METER/////////////
/////////////////////////////////
void SpeedoFunc(){
VoltSpeedo = analogRead(Speedometre) * 3.3 / 4096.0;
Speedo = VoltSpeedo * 208 / 3.3;
//Serial.print("Voltage speedo: ");
//Serial.println(VoltSpeedo);
//Serial.print("Vitesse: ");
//Serial.println(Speedo);
};


void AutoLockRun(){
  SpeedoFunc();
  if (Speedo >= 208 && !StateAutoLockRun)
  {
    LockRelay();
    StateAutoLockRun = true;
  } 
};

//////////////////////////////
////////RPM METER/////////////
//////////////////////////////
void RpmFunc(){
VoltTachy = analogRead(tachymetre) * 3.3 / 4096.0;
Rpm = VoltTachy * 7000 / 3.3;
//Serial.print("Voltage Tachy: ");
//Serial.println(VoltTachy);
//Serial.print("Rpm: ");
//Serial.println(Rpm);
};


//////////////////////////////////////
////////DOOR SWITCH ALARM/////////////
//////////////////////////////////////
void DoorSwitchFunc(){
VoltDoorSwitch = analogRead(doorSwitchPin) * 3.3 / 4096.0;
doorSwitch = VoltDoorSwitch * 16.5 / 3.3;
};

void checkOpenCar(){
  if (doorSwitch >= 10 && !carOpen)
  {
    carOpen = true;
    NotifUnlockFunc(1);
  }
};

/////////////////////////////////////
////////AUTO SHUTDOWN ACCY IGN///////
/////////////////////////////////////
void AutoShutdownAccyIgn(){
  if (IgnitionStarted || AccyStarted)
  {
      if (!AS_StartTimer)
      {
        AS_previousMillis = millis();
        AS_StartTimer = true;
      }
      if (AS_StartTimer)
      {
          unsigned long AS_currentMillis = millis();
          if (AS_currentMillis - AS_previousMillis >= 600000) { // 600000ms = 10 minutes
              Serial.print("Auto ShutDown Accy/Ignition");
              Ignition3(LOW);
              Accy(LOW);
              Ignition1(LOW);
              BlinkSwitchLed = false;
              engineSwitchLed(0);
              IgnitionStarted = false;
              AccyStarted = false;
              AS_StartTimer = false;
        }
      }
  }
  else if (!IgnitionStarted && !AccyStarted && AS_StartTimer)
  {
    AS_StartTimer = false;
  }
};
//////////////////////////
////////SLEEP MODE////////
//////////////////////////
void sleepModeFunc(){
  if (carOpen) {
    if (SleepModeReady)
    {
      // Full Clock CPU
      setCpuFrequencyMhz(240);
      SleepModeReady = false;
      Serial.println("########### Power Mode");
    } 
    else if (!SleepModeReady && SM_startTimer) {
      SM_startTimer = false;
    }
  }
  else if (!carOpen && !IgnitionStarted && !SleepModeReady)
  {
        if (!SM_startTimer)
        {
          SM_previousMillis = millis();
          SM_startTimer = true;
        }
        if (SM_startTimer)
        {        
          unsigned long SM_currentMillis = millis();
          if (SM_currentMillis - SM_previousMillis >= 600000) { // 600000ms = 10 minutes
              // Clock CPU to Energize save
              setCpuFrequencyMhz(80);
              Serial.println("########### Sleep Mode");
              SM_startTimer = false;
              SleepModeReady = true;
          }
        }
  }
};