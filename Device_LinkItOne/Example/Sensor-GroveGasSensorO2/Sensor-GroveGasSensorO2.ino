#include <math.h>
// sensor PIN: A2
float WarningValue= 19.5;  //The minimum sate concentration of O2 in air  
void setup()
{
   Serial.begin(115200);                //Start the Serial connection
 
}
void loop()
{
//long unsigned a;
float sensorValue;
float sensorVoltage; 
float Value_O2;
sensorValue = analogRead(A2);
sensorVoltage =(sensorValue/1024)*5.0;
sensorVoltage = sensorVoltage/201*10000;
Value_O2 = sensorVoltage/7.43;
 
Serial.print("Concentration of O2 is ");
Serial.print(Value_O2,1);
Serial.println("%");
 

delay(1000);
}
