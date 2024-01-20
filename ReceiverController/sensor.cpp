/*********
  THIROUX Yannis 
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  ©2023 THIROUX Yannis, tous droits réservés.
*********/

#include <Arduino.h>
#include <sensor.h>
#include <relay.h>

unsigned long previousMillis = 0;

extern bool EngineStarted;
extern bool IgnitionStarted;
extern bool AccyStarted;

//PIN INPUT Voltage sensor
const int voltageSensor = 35;        //Sur PIN15
float VoltT;
extern float Voltage;
float Offset = -0.18;

void initpinsensor(){ 
  pinMode(voltageSensor, INPUT);
};


// ***** function calcul la moyen de la valeur de l'analogRead******
float getVPP()
{
  float result;
  int readValue;                // value read from the sensor
  int maxValue = 0;             // store max value here
  int minValue = 4096;          // store min value here ESP32 ADC resolution
  
   uint32_t start_time = millis();
   while((millis()-start_time) < 1000) //sample for 1 Sec
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
  //VoltT = analogRead(voltageSensor)* 3.3 / 4095; //Calculated Voltage (4095 max , ADC resolution 12bits )
  VoltT = getVPP();
  Serial.println(VoltT);
  Voltage = (VoltT - Offset) * 16.5 / 3.3; //Calculated Voltage in V
};

void checkEngineStart(){ 
  if (Voltage >= 13)
  {
    EngineStarted = true;
    IgnitionStarted = true;
    Ignition3(HIGH);
    Accy(HIGH);
    Ignition1(HIGH);
    Serial.println("Engine is started.");
  }
  else {
    EngineStarted = false;
    Serial.println("Engine isn't started.");
  }
};

void AutoShutdownAccyIgn(){
  if (IgnitionStarted || AccyStarted)
  {
      if (!EngineStarted)
      Serial.print("Shutdonw in 10 minutes");
      {
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= 600000) {
              previousMillis = currentMillis;
              Serial.print("Auto ShutDown Accy Ign");
              Ignition3(LOW);
              Accy(LOW);
              Ignition1(LOW);
              IgnitionStarted = false;
              AccyStarted = false;
              previousMillis = 0;
        }
      }
      if (EngineStarted && previousMillis != 0)
      {
          previousMillis = 0;
      }
  }
  
};