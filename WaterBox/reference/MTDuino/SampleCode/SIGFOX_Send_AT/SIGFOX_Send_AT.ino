#include <Arduino.h>                  // required before wiring_private.h
#include "wiring_private.h"           // pinPeripheral() function
int LED_POW=3;                        // sigfox status LED pin D3
int RF_PWEN=5;                        // sigfox power control pin D5

unsigned long previousMillis = 0;    // Time Counter
const long interval = 10000;          // Unit: ms. 

void setup() {
  SerialUSB.begin(9600);          // Set Terminal Baudrate: 9600bps
  Serial1.begin(9600);             // Set sigfox Baudrate:9600bps
  pinMode(LED_POW,OUTPUT);        // Set LED_POW pin as output
  pinMode(RF_PWEN,OUTPUT);       // Set RF_PWEN pin as output
  digitalWrite(LED_POW,HIGH);    // Set LED_POW to High
  digitalWrite(RF_PWEN,HIGH);    // Set RF_PWEN to High, and Sigfox module power ON

}
void loop()
{
  unsigned long currentMillis = millis();
   //sigfox AT Command
  if (currentMillis - previousMillis >= interval)
  {
    sigfox_atcommand_tx("AT$RCZ?\r");          //sigfox AT Command: get RCZ
    sigfox_atcommand_tx("AT$ID?\r");           //sigfox AT Command: Get ID
    sigfox_atcommand_tx("AT$PAC?\r");          //sigfox AT Command: Get PAC
    delay(100);
  
     previousMillis = currentMillis;
     sigfox_atcommand_tx("AT$SF=abcdefg\r");     // Send Data to backend function
  }
}

void sigfox_atcommand_tx(char *wBuffer)   
{
  Serial1.print(wBuffer);                            // Send Data to backend                 
  sigfox_atcommand_rx();                            // Get feedback and show on Terminal function
}

void sigfox_atcommand_rx()
{
  char temp;
  while(Serial1.available() <= 0);
  while(Serial1.available() > 0)
  {
    temp = Serial1.read();
    SerialUSB.print(temp);
    delay(10);
  }
}


