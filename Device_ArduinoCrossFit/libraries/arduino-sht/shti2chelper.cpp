/*
 *  Copyright (c) 2015, Johannes Winkelmann <jw@smts.ch>
 *  All rights reserved.
 *  
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *      * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *      * Neither the name of the <organization> nor the
 *        names of its contributors may be used to endorse or promote products
 *        derived from this software without specific prior written permission.
 *  
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <Arduino.h>
#include <Wire.h>

#include "shti2chelper.h"

bool ShtI2CHelper::readFromI2C(uint8_t i2cAddress, const uint8_t* i2cCommand, uint8_t commandLength, uint8_t* data, uint8_t dataLength)
{
  Wire.beginTransmission(i2cAddress);
  for (int i = 0; i < commandLength; ++i) {
    if (Wire.write(i2cCommand[i]) != 1) {
      return false;
    }
  }  
  if (Wire.endTransmission(false) != 0) {
    Serial.println("NACK");
    return false;
  }

  Wire.requestFrom(i2cAddress, dataLength);

  // there should be no reason for this to not be ready, since we're using clock stretching mode,
  // but just in case we'll try a few times
  uint8_t tries = 1;
  while (Wire.available() < dataLength) {
    delay(1);
    if (tries++ >= MAX_READ_TRIES) {
      return false;
    }
  }
  
  for (int i = 0; i < dataLength; ++i) {
    data[i] = Wire.read();
  }
  return true;
}


