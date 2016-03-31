
#define gasMQ2Pin A5


class deviceMQ2 {    
  public:
    deviceMQ2() {
      
    }

    float readValue() {
      return analogRead(gasMQ2Pin);
    }
};

deviceMQ2 sensorMQ2;

void setup() {
  Serial.begin(9600);
  Serial.println("MQ2 test!");

}

void loop() {
  Serial.println(sensorMQ2.readValue());
  delay(2000);    
}  
