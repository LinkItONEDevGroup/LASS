
#define gasMQ4Pin A4


class deviceMQ4 {    
  public:
    deviceMQ4() {
      
    }

    float readValue() {
      return analogRead(gasMQ4Pin);
    }
};

deviceMQ4 sensorMQ4;

void setup() {
  Serial.begin(9600);
  Serial.println("MQ4 test!");

}

void loop() {
  Serial.println(sensorMQ4.readValue());
  delay(2000);
    
}  
