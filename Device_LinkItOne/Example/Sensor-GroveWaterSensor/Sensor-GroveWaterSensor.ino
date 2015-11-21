/*macro definition of water sensor and the buzzer*/
//STATUS: not confirmed work. debugging.
#define WATER_SENSOR 7
float sensorValue[0];
float get_sensor_data_water(){
  if(digitalRead(WATER_SENSOR)==HIGH){
    return 0;
  }
  else{
    return 1;
  }
}
void setup()
{
  pinMode(WATER_SENSOR, INPUT);  
  Serial.begin(115200); 
}

void loop()
{
  sensorValue[0] = get_sensor_data_water();  
  if(sensorValue[0]>0){
    Serial.println("In Water");
  }else{
    Serial.println("No Water");
  }  
  delay(1000);
}

