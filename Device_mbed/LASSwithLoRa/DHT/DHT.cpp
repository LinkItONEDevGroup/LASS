/*
 *  DHT Library for  Digital-output Humidity and Temperature sensors
 *
 *  Works with DHT11, DHT22
 *             SEN11301P,  Grove - Temperature&Humidity Sensor     (Seeed Studio)
 *             SEN51035P,  Grove - Temperature&Humidity Sensor Pro (Seeed Studio)
 *             AM2302   ,  temperature-humidity sensor
 *             HM2303   ,  Digital-output humidity and temperature sensor
 *
 *  Copyright (C) Wim De Roeve
 *                based on DHT22 sensor library by HO WING KIT
 *                Arduino DHT11 library 
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documnetation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to  whom the Software is
 * furished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "DHT.h"

#define DHT_DATA_BIT_COUNT 41

DHT::DHT(PinName pin,int DHTtype) {
    _pin = pin;
    _DHTtype = DHTtype;
    _firsttime=true;
}

DHT::~DHT() {
}

void DHT::printErr(int ErrType) {
 switch (ErrType) {
   case 0: printf("ERROR_NONE"); break;
   case 1: printf("BUS_BUSY"); break;
   case 2: printf("ERROR_NOT_PRESENT"); break;
   case 3: printf("ERROR_ACK_TOO_LONG"); break;
   case 4: printf("ERROR_SYNC_TIMEOUT"); break;
   case 5: printf("ERROR_DATA_TIMEOUT"); break;
   case 6: printf("ERROR_CHECKSUM"); break;
   case 7: printf("ERROR_NO_PATIENCE"); break;

 } 
 printf("\r\n");   
}    

int DHT::readData() {
    int i, j, retryCount,b;
    unsigned int bitTimes[DHT_DATA_BIT_COUNT];

    eError err = ERROR_NONE;
    time_t currentTime = time(NULL);

    DigitalInOut DHT_io(_pin);

    for (i = 0; i < DHT_DATA_BIT_COUNT; i++) {
        bitTimes[i] = 0;
    }

    if (!_firsttime) {
        if (int(currentTime - _lastReadTime) < 2) {
            err = ERROR_NO_PATIENCE;
            return err;
        }
    } else {
        _firsttime=false;
        _lastReadTime=currentTime;
    }
    retryCount = 0;

    do {
        if (retryCount > 125) {
            err = BUS_BUSY;
            return err;
        }
        retryCount ++;
        wait_us(2);
    } while ((DHT_io==0));


    DHT_io.output();
    DHT_io = 0;
    wait_ms(5);
    DHT_io = 1;
    wait_us(20);
    DHT_io.input();

    retryCount = 0;
    do {
        if (retryCount > 40)  {
            err = ERROR_NOT_PRESENT;
            return err;
        }
        retryCount++;
        wait_us(1);
    } while ((DHT_io==1));

    if (err != ERROR_NONE) {
        return err;
    }

    wait_us(80); //step 2 wait for High
   
    for (i = 0; i < 5; i++) {
        for (j = 0; j < 8; j++) {

            retryCount = 0;
            do {
                if (retryCount > 75)  {
                    err = ERROR_DATA_TIMEOUT;
                    return err;
                }
                retryCount++;
                wait_us(1);
            } while (DHT_io == 0);
            wait_us(40);
            bitTimes[i*8+j]=DHT_io;

            int count = 0;
            while (DHT_io == 1 && count < 100) {
                wait_us(1);
                count++;
            }
        }
    }
    DHT_io.output();
    DHT_io = 1;
    for (i = 0; i < 5; i++) {
        b=0;
        for (j=0; j<8; j++) {
            if (bitTimes[i*8+j+1] > 0) {
                b |= ( 1 << (7-j));
            }
        }
        DHT_data[i]=b;
    }

    if (DHT_data[4] == ((DHT_data[0] + DHT_data[1] + DHT_data[2] + DHT_data[3]) & 0xFF)) {
        _lastReadTime = currentTime;
        _lastTemperature=CalcTemperature();
        _lastHumidity=CalcHumidity();

    } else {
        err = ERROR_CHECKSUM;
    }

    return err;

}

float DHT::CalcTemperature() {
    int v;
    float outvalue;
    switch (_DHTtype) {
        case DHT11:
            v = DHT_data[2];
            return float(v);
        case DHT22:
            v = DHT_data[2] & 0x7F;
            v *= 256;
            v += DHT_data[3];
            outvalue=(float) v/10;
            if (DHT_data[2] & 0x80)
                outvalue *= -1;
            return outvalue;
    }
    return 0;
}

float DHT::ReadHumidity() {
    return _lastHumidity;
}

float DHT::ConvertCelciustoFarenheit(float celsius) {
    return celsius * 9 / 5 + 32;
}

float DHT::ConvertCelciustoKelvin(float celsius) {
    return celsius + 273.15;
}

// dewPoint function NOAA
// reference: http://wahiduddin.net/calc/density_algorithms.htm
float DHT::CalcdewPoint(float celsius, float humidity) {
    float A0= 373.15/(273.15 + celsius);
    float SUM = -7.90298 * (A0-1);
    SUM += 5.02808 * log10(A0);
    SUM += -1.3816e-7 * (pow(10, (11.344*(1-1/A0)))-1) ;
    SUM += 8.1328e-3 * (pow(10,(-3.49149*(A0-1)))-1) ;
    SUM += log10(1013.246);
    float VP = pow(10, SUM-3) * humidity;
    float T = log(VP/0.61078);   // temp var
    return (241.88 * T) / (17.558-T);
}

// delta max = 0.6544 wrt dewPoint()
// 5x faster than dewPoint()
// reference: http://en.wikipedia.org/wiki/Dew_point
float DHT::CalcdewPointFast(float celsius, float humidity)
{
        float a = 17.271;
        float b = 237.7;
        float temp = (a * celsius) / (b + celsius) + log(humidity/100);
        float Td = (b * temp) / (a - temp);
        return Td;
}

float DHT::ReadTemperature(eScale Scale) {
    if (Scale == FARENHEIT)
        return ConvertCelciustoFarenheit(_lastTemperature);
    else if (Scale == KELVIN)
        return ConvertCelciustoKelvin(_lastTemperature);
    else
        return _lastTemperature;
}

float DHT::CalcHumidity() {
    int v;
    float outvalue;
    
    switch (_DHTtype) {
        case DHT11:
            v = DHT_data[0];
            return float(v);
        case DHT22:
            v = DHT_data[0];
            v *= 256;
            v += DHT_data[1];
            outvalue=(float) v/10;
            return outvalue;
    }
    return 0;
}


