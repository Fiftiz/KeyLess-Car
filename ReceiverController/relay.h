/*********
  THIROUX Yannis 
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  ©2023 THIROUX Yannis, tous droits réservés.
*********/
#ifndef relay_h
#define relay_h

#include <Arduino.h>

void initPin();
void initPosition();
void UnLockRelay();
void LockRelay();
void Starter(uint8_t value);
void Ignition3(uint8_t value);
void Ignition1(uint8_t value);
void Accy(uint8_t value);
void engineSwitchLed(int value);
void diagMode(uint8_t value);

extern void NotifUnlockFunc(int value);

#endif