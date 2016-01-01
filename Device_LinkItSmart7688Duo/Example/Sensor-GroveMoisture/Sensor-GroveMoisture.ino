int sensorPin = A1;    // select the input pin for the potentiometer

float sensorValue[0];

float get_sensor_data_moisture(){
  // read the value from the sensor:
  return analogRead(sensorPin);      
}

void setup() {
  // declare the ledPin as an OUTPUT:
   Serial.begin(115200);  
}
 
void loop() {
  sensorValue[0]=get_sensor_data_moisture();
  Serial.print("sensor = " );                       
  Serial.println(sensorValue[0]);                   
  delay(1000);          
}
