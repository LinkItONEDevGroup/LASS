/*
	Version: 0.1.00
	Purpose: This sample use Ameba and Gemtek LoRa module, 
             to test your module is ready or not.
             Simply send an "AT" command to LoRa module, and it should
             give you "OK" response, otherwise your module is not ready yet,
             you should check your wire connection and pin setting.
    
    
	History:
	0.1.00  by Hu-Cheng Lee (Jack)  12/04/2016  
*/

#include <SoftwareSerial.h>
#define BUFF_LENGTH 50

SoftwareSerial lora(0, 1); // RX, TX
    
char readBuffer[BUFF_LENGTH];

void setup() {
    // put your setup code here, to run once:
    Serial.begin(38400);  
    lora.begin(9600); // lora
}

void loop() {
    
    // put your main code here, to run repeatedly:
    Serial.println("Ready for some LoRa?");
    lora.println("AT");
    delay(500);
	ReadFromLoRa();
    if(readBuffer[0] == 'O' && readBuffer[1] == 'K'){
        Serial.println("Yes, let's push some data!");
    }
    else{
        Serial.println("No, LoRa is not ready.");  
    }
    delay(3000);
}
void ReadFromLoRa(){
	int wordCount = 0;
	memset(readBuffer, 0, sizeof(readBuffer));
    for(wordCount = 0;lora.available() > 0;wordCount++){
        readBuffer[wordCount] = lora.read();
        delay(250);
    }     
}
