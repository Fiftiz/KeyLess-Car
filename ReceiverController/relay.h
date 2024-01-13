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
void StartEngine();
void UnLockRelay();
void LockRelay();
void IgnitionOFFduringStart(uint8_t value);
void IgnitionONduringStart(uint8_t value);

#endif