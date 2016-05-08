#include "serialReadPMValue.h"


class devicePMS3003 {    
  public:
    devicePMS3003() {
      
    }

    long readValue(int pmType) {
        boolean readFailed = 1;
        long pm;
        
        while(readFailed==1) {
          switch (pmType) {
            case 25:
              pm = dataPM25();
              break;
            case 10:
              pm = dataPM10();
              break;
            case 100:
              pm = dataPM100();
              break;
            }
            if(pm!=999999999 && pm>0) readFailed=0;
        }
          
        return pm;
    }

    long dataPM25() {
      long pmcf25=999999999;
      long pmat25=999999999;
      char buf[50];
      int count = 0;
      unsigned char c;
      unsigned char high;
    
      delay(1000);      
      
      while (Serial.available()) {
          c = Serial.read();
          //Serial.write(c);
          if((count==0 && c!=0x42) || (count==1 && c!=0x4d)){
            Serial.print(" CheckFailed ");
            break;
          }
          if(count > 15){
            break;
          }
          else if(count == 4 || count == 6 || count == 8 || count == 10 || count == 12 || count == 14) {
            high = c;
          }
          else if(count == 7){
            pmcf25 = 256*high + c;
          }
          else if(count == 13){
            pmat25 = 256*high + c;
          }
          count++;
        }
        while(Serial.available()) Serial.read();
        delay(500);
        return pmcf25;
    }

    long dataPM10() {
      long pmcf10=999999999;
      long pmat10=999999999;
      char buf[50];
      int count = 0;
      unsigned char c;
      unsigned char high;
    
      delay(1000);      
      
      while (Serial.available()) {
          c = Serial.read();
          //Serial.write(c);
          if((count==0 && c!=0x42) || (count==1 && c!=0x4d)){
            Serial.print(" CheckFailed ");
            break;
          }
          if(count > 15){
            break;
          }
          else if(count == 4 || count == 6 || count == 8 || count == 10 || count == 12 || count == 14) {
            high = c;
          }
          else if(count == 5){
            pmcf10 = 256*high + c;
          }
          else if(count == 11){
            pmat10 = 256*high + c;
          }
          count++;
        }
        while(Serial.available()) Serial.read();
        delay(500);
        return pmcf10;
    }

    long dataPM100() {

      long pmcf100=999999999;
      long pmat100=999999999;
      char buf[50];
      int count = 0;
      unsigned char c;
      unsigned char high;
    
      delay(1000);      
      
      while (Serial.available()) {
          c = Serial.read();
          //Serial.write(c);
          if((count==0 && c!=0x42) || (count==1 && c!=0x4d)){
            Serial.print(" CheckFailed ");
            break;
          }
          if(count > 15){
            break;
          }
          else if(count == 4 || count == 6 || count == 8 || count == 10 || count == 12 || count == 14) {
            high = c;
          }
          else if(count == 9){
            pmcf100 = 256*high + c;
          }
          else if(count == 15){
            pmat100 = 256*high + c;
          }
          count++;
        }
        while(Serial.available()) Serial.read();
        delay(500);
        return pmcf100;
    }
};

devicePMS3003 sensorPMS3003;

void setup() {
  Serial.begin(9600);
  Serial.println("PMS3003 test!");

}

void loop() {
  Serial.print(sensorPMS3003.readValue(25)); Serial.print(" / ");
  Serial.print(sensorPMS3003.readValue(10)); Serial.print(" / ");
  Serial.println(sensorPMS3003.readValue(100)); 
  delay(2000);
    
}  
