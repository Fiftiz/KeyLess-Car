/*********
  THIROUX Yannis 
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  ©2023 THIROUX Yannis, tous droits réservés.
*********/
#ifndef sensor_h
#define sensor_h

#include <Arduino.h>

void voltage();
void initpinsensor();
void checkEngineStart();
float getVPP();
void AutoLockRun();
void AutoShutdownAccyIgn();
void sleepModeFunc();
void SpeedoFunc();
void RpmFunc();
void DoorSwitchFunc();
void checkOpenCar();

extern void NotifUnlockFunc(int value);

#endif