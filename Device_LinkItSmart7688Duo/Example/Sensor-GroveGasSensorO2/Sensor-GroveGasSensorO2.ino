#include <math.h>
// sensor PIN: A2
unsigned long currentTime = 0;  // current loop time
float sensorValue[0];

float get_sensor_data_gas_o2(){
  float sensorValue;
  float sensorVoltage; 
  float Value_O2;

  sensorValue = analogRead(A2);
  sensorVoltage =(sensorValue/1024)*5.0;
  sensorVoltage = sensorVoltage/201*10000;
  Value_O2 = sensorVoltage/7.43;
  return Value_O2;
  
}
void setup()
{
   Serial.begin(115200);                //Start the Serial connection
 
}

void loop()
{
  currentTime = millis();
  Serial.print("current tick= ");
  Serial.print(currentTime);
   
  sensorValue[0] = get_sensor_data_gas_o2();
  
  Serial.print(",Concentration of O2 is ");
  Serial.print(sensorValue[0],1);
  Serial.println("%");
   
  delay(10000);
}


