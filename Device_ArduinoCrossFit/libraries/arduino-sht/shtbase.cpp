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

#include <Wire.h>
#include <Arduino.h>

#include "shtbase.h"
#include "shti2chelper.h"

ShtBase::ShtBase(uint8_t i2cAddress, 
                                         uint16_t i2cCommand,
                                         float a, float b, float c,
                                         float x, float y)
  : mI2CAddress(i2cAddress), 
    mI2CCommand(i2cCommand),
    mHumidity(HUMIDITY_INVALID),
    mTemperature(TEMPERATURE_INVALID),
    mA(a), mB(b), mC(c),
    mX(x), mY(y)
{
}

float ShtBase::getHumidity() const
{
  return mHumidity;
}

float ShtBase::getTemperature() const
{
  return mTemperature;
}

bool ShtBase::readSample()
{
  uint8_t data[EXPECTED_DATA_SIZE];
  uint8_t cmd[CMD_SIZE];
  
  cmd[0] = mI2CCommand >> 8;
  cmd[1] = mI2CCommand & 0x00ff;
  
  if (!ShtI2CHelper::readFromI2C(mI2CAddress, cmd, CMD_SIZE, data, EXPECTED_DATA_SIZE)) {
    return false;
  }
  
  // -- Important: this is assuming 2-byte data, 1-byte

  // check CRC for both RH and T
  if (crc8(data+0, 2) != data[2] || crc8(data+3, 2) != data[5]) {
    return false;
  }

  // convert to Temperature/Humidity
  uint16_t val;
  val = (data[0] << 8) + data[1];
  mTemperature = mA + mB * (val / mC);
  
  val = (data[3] << 8) + data[4];
  mHumidity = mX * (val / mY);
  
  return true;
}


uint8_t ShtBase::crc8(const uint8_t* data, uint8_t len)
{
  // adapted from SHT21 sample code from http://www.sensirion.com/en/products/humidity-temperature/download-center/

  uint8_t crc = 0xff;
  uint8_t byteCtr;
  for (byteCtr = 0; byteCtr < len; ++byteCtr) {
    crc ^= (data[byteCtr]);
    for (uint8_t bit = 8; bit > 0; --bit) {
      if (crc & 0x80) {
        crc = (crc << 1) ^ 0x31;
      } else {
        crc = (crc << 1);
      }
    }
  }
  return crc;
}
