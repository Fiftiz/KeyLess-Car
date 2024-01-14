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

extern bool EngineStarted;
extern bool IgnitionStarted;

//PIN INPUT Voltage sensor
const int voltageSensor = 35;        //Sur PIN15
float VoltT;
extern float Voltage;
float Offset = 0;

void initpinsensor(){ 
  pinMode(voltageSensor, INPUT);
};

void voltage(){ 
  //VoltT = analogRead(voltageSensor)* 3.3 / 4095; //Calculated Voltage (4095 max , ADC resolution 12bits )
  VoltT = getVPP();
  Voltage = (VoltT - Offset) * 16.5 / 3.3; //Calculated Voltage in V
};

void checkEngineStart(){ 
  if (Voltage >= 13)
  {
    EngineStarted = true;
    IgnitionStarted = true;
    Serial.println("Engine is started.");
  }
  else {
    EngineStarted = false;
    Serial.println("Engine isn't started.");
  }
};

// ***** function calcul la moyen de la valeur de l'analogRead******
float getVPP()
{
  float result;
  int readValue;                // value read from the sensor
  int maxValue = 0;             // store max value here
  int minValue = 4096;          // store min value here ESP32 ADC resolution
  
   uint32_t start_time = millis();
   while((millis()-start_time) < 2000) //sample for 2 Sec
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