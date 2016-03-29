
#define waterPin A3


class deviceFC37 {    
  public:
    deviceFC37() {
      
    }

    float readValue() {
       int sensorRead = analogRead(waterPin);
       //return (sensorRead*100)/1023;
       int ratio = 100-(sensorRead/10);
       if(ratio<0) {
          return 0;
       }else if(ratio>100) {
          return 100;
       }else{
          return ratio;
       }
    }
};

deviceFC37 sensorFC37;

void setup() {
  Serial.begin(9600);
  Serial.println("FC-37 test!");

}

void loop() {
  Serial.println(sensorFC37.readValue());
  delay(2000);
    
}  
