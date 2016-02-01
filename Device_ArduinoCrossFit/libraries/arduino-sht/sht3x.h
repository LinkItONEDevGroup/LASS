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

#ifndef SHT3X_H
#define SHT3X_H

#include "shtbase.h"

class SHT3X : public ShtBase
{
public:
  enum SHT3X_I2C_ADDRESS {
    I2C_ADDRESS_44  = 0x44,
    I2C_ADDRESS_45  = 0x45
  };
  
  enum SHT3X_ACCURACY {
    ACCURACY_HIGH   = 0x2C06,
    ACCURACY_MEDIUM = 0x2C0D,
    ACCURACY_LOW    = 0x2C10
  };
  
  SHT3X() : ShtBase(I2C_ADDRESS_44, ACCURACY_HIGH,
                    -45, 175, 65535,
                    100, 65535)
    {}
  
  void setAccuracy(SHT3X_ACCURACY newAccuracy) 
    { mI2CCommand = newAccuracy; }
  void setAddress(SHT3X_I2C_ADDRESS newAddress)
    { mI2CAddress = newAddress; }
};


#endif /* #ifdef SHT3X_H */
