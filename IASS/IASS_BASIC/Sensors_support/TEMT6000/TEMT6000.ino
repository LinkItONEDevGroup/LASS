
#define lightPin A1


class deviceLight {    
  public:
    deviceLight() {
      
    }

    float readValue() {
      float volts = analogRead(lightPin) * 5.0 / 1024.0;
      float amps = volts / 10000.0;  // across 10,000 Ohms
      float microamps = amps * 1000000;
      return microamps * 2.0;  // lux value
    }
};

deviceLight sensorLight;

void setup() {
  Serial.begin(9600);
  Serial.println("Light test!");

}

void loop() {
  Serial.println(sensorLight.readValue());
  delay(2000);
    
}  
