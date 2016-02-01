#include <Wire.h>

#include "shtc1.h"

SHTC1 shtc1;

void setup() {
  // put your setup code here, to run once:

  Wire.begin();  
  Serial.begin(9600);
  
  delay(1000); // let serial console settle
}

void loop() {
  // put your main code here, to run repeatedly:

  shtc1.readSample();
  Serial.print("SHTC1:\n");
  Serial.print("  RH: ");
  Serial.print(shtc1.getHumidity(), 2);
  Serial.print("\n");
  Serial.print("  T:  ");
  Serial.print(shtc1.getTemperature(), 2);
  Serial.print("\n");

  delay(1000);
}
