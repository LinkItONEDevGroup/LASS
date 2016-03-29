#include "DHT.h"

DHT dht(6, DHT22);


class deviceDHT {    
  public:
    deviceDHT() {
      dht.begin();  
    }
    float v;
    float readValue(char typeT) {
      if(typeT=='T') {
          v = dht.readTemperature();
      }else if(typeT=='F') { 
          v = dht.readTemperature(true);
      }else if(typeT=='H') {
          v = dht.readHumidity();
      }

      if (isnan(v)) {    
        Serial.println("Failed to read from DHT sensor!");
        return -9999;
      }else{
        return v;
      }
    }
};

deviceDHT sensorDHT;

void setup() {
  Serial.begin(9600);
  Serial.println("DHTxx test!");
}

void loop() {
  Serial.print("Temperature: "); Serial.print(sensorDHT.readValue('T'));
  Serial.print(" / "); 
  Serial.print("Humandity: "); Serial.println(sensorDHT.readValue('H'));
  delay(2000);
    
}  
