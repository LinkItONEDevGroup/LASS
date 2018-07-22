//
// File Name: SIGFOX_Terminal_SendAT.ino
//----------------------------------------------------------------------------
//
// Author: Albert Lien
// Create Date: 02/14/2018 
// Revision: 1.01
//
// Description:
//    This code is designed to send data to Sigfox backend by AT command via Terminal. 
//    Using command “AT$SF” to make sure device is ready to connect Sigfox backend. 
//    It also can used AT command to get Sigfox registered information ID and PAC from Terminal, 
//    show on screen.
// 
//    1). Send data to Sigfox backend.
//    2). Sigfox Module Power control: RF_PWEN(D5)
//    6). Sigfox UART Sercom: SERCOM0 D1(UART TX) D0(UART RX)                                 
// 
// Target Board: Arduino M0 Pro
// Target Device: ATSAMD21G18
//
// Copyright (c) 
// 
// Designed by Mighty Net, http://http://www.mightynet.co/ 
//-----------------------------------------------------------------------------

#include <Arduino.h>                      // required before wiring_private.h
#include "wiring_private.h"               // pinPeripheral() function
int LED_POW=3;                            // sigfox status LED pin D3
int RF_PWEN=5;                            // sigfox power control pin D5
int count, i, INT;
char value;

void setup() 
{
    SerialUSB.begin(9600);                // Set Terminal Baudrate: 9600bps
    Serial1.begin(9600);                   // Set sigfox Baudrate:9600bps
    pinMode(LED_POW,OUTPUT);               // Set LED_POW pin as output 
    pinMode(RF_PWEN,OUTPUT);               // Set RF_PWEN pin as output 
    digitalWrite(LED_POW,HIGH);            // Set LED_POW to High
    digitalWrite(RF_PWEN,HIGH);            // Set RF_PWEN to High, and Sigfox module power ON
}

void loop()
{
    INT = SerialUSB.available();          
                                          
    if(INT>1)
    {      
        for(int i = 0; i < INT; i ++)
        {
            value = SerialUSB.read();    
            Serial1.print(value);                
        } 
        Serial1.print("\r");
    }  
    delay(100);
    count=Serial1.available(); 
    if(count>1)
    {      
        for(int i = 0; i < count; i ++)
        {
            value = Serial1.read();
            SerialUSB.print(value);        
        } 
    }
    delay(100);
}
