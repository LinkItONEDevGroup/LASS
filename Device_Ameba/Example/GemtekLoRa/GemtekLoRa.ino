#include <SoftwareSerial.h>
#define BUFF_LENGTH 30

SoftwareSerial lora(0, 1); // RX, TX

char readBuffer[BUFF_LENGTH];
char input;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(38400);  
  lora.begin(9600); // lora
}

void loop() {
  int wordCount = 0;
  // put your main code here, to run repeatedly:
  Serial.println("Ready for some LoRa?");
  lora.println("AT");
  for(wordCount = 0;lora.available() > 0;wordCount++){
    input = lora.read();
    readBuffer[wordCount] = input;
  }  
  if(readBuffer[0] == 'O' && readBuffer[1] == 'K'){
    Serial.println("Yes, let's push some data!");
  }
  else{
    Serial.println("No, LoRa is not ready.");  
  }
  memset(readBuffer, 0, sizeof(readBuffer));
  delay(5000);
}
