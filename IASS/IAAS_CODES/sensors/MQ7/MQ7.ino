
#define gasMQ7Pin A5


class deviceMQ7 {    
  public:
    deviceMQ7() {
      
    }

    float readValue() {
      return analogRead(gasMQ7Pin);
    }
};

deviceMQ7 sensorMQ7;

void setup() {
  Serial.begin(9600);
  Serial.println("MQ7 test!");

}

void loop() {
  Serial.println(sensorMQ7.readValue());
  delay(2000);
    
}  
