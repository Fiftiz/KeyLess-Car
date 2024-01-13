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
float VoltP;
extern float Voltage;
float Offset = 0;

void initpinsensor(){ 
  pinMode(voltageSensor, INPUT);
};

void voltage(){ 
  VoltP = analogRead(voltageSensor)* 3.3 / 4095; //Calculated Voltage (4095 max , ADC resolution 12bits )
  Voltage = (VoltP - Offset) * 16.5 / 3.3; //Calculated Voltage in V
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