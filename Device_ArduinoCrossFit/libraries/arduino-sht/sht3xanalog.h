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

#ifndef SHT3XANALOG_H
#define SHT3XANALOG_H

#include <Arduino.h>
#include <math.h>
#include <inttypes.h>

class SHT3XAnalog
{
public:
  SHT3XAnalog(uint8_t humidityPin, uint8_t temperaturePin) 
    : mHumidityAdcPin(humidityPin), mTemperatureAdcPin(temperaturePin)
  {
  }
  
  double readHumidity()
    { return -12.5 + 125 * (analogRead(mHumidityAdcPin) / (double)MAX_ADC); }
  double readTemperature()
    { return -66.875 + 218.75 * (analogRead(mTemperatureAdcPin) / (double)MAX_ADC); }

private:
  uint8_t mHumidityAdcPin;
  uint8_t mTemperatureAdcPin;
  
  static const uint16_t MAX_ADC = 1023; // for the 10-bit default
};

#endif /* SHT3XANALOG_H */
