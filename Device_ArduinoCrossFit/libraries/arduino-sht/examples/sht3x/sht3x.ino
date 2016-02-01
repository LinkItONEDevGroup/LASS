#include <Wire.h>

#include "sht3x.h"

SHT3X sht3x;

void setup() {
  // put your setup code here, to run once:

  sht3x.setAddress(SHT3X::I2C_ADDRESS_45);
  sht3x.setAccuracy(SHT3X::ACCURACY_MEDIUM);
  Wire.begin();  
  Serial.begin(9600);
  
  delay(1000); // let serial console settle
}

void loop() {
  // put your main code here, to run repeatedly:
    
  sht3x.readSample();
  Serial.print("SHT3x:\n");
  Serial.print("  RH: ");
  Serial.print(sht3x.getHumidity(), 2);
  Serial.print("\n");
  Serial.print("  T:  ");
  Serial.print(sht3x.getTemperature(), 2);
  Serial.print("\n");

  delay(1000);
}
