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

#ifndef SHTBASE_H
#define SHTBASE_H

#include <inttypes.h>

/*
 * Base class for SHTC1, SHTW1 and SHT3x sensors
 *
 * this is assuming 2-byte data, 1-byte CRC for each RH and T
 * Temperature first, RH second
 */
class ShtBase
{
public:
  /*
   * Note: subclasses should never expose the i2c command
   */
  ShtBase(uint8_t i2cAddress, 
                      uint16_t i2cCommand,
                      float a, float b, float c,
                      float x, float y);

  float getHumidity() const;
  float getTemperature() const;
  
  bool readSample();

protected:
  uint8_t mI2CAddress;
  uint16_t mI2CCommand;

private:
  static uint8_t crc8(const uint8_t* data, uint8_t len);


  float mHumidity;
  float mTemperature;

  // -- conversion parameters
  // T_OUT = A + B * (T_IN  / C)
  float mA = 0;
  float mB = 0;
  float mC = 1;
  // RH_OUT = X * (RH_IN / Y)
  float mX = 0;
  float mY = 1;
  
  static const uint8_t MAX_READ_TRIES      = 5;
  static const uint8_t CMD_SIZE            = 2;
  static const uint8_t EXPECTED_DATA_SIZE  = 6;  
  static const int16_t HUMIDITY_INVALID    = -1;
  static const int16_t TEMPERATURE_INVALID = -999;
};

#endif /* SHTBASE_H */
