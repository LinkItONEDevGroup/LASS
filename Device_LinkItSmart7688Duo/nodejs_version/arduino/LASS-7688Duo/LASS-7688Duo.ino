#include <SoftwareSerial.h>
#include <dht.h>   

//設定pin腳
#define dht_dpin 15 //Dht用Pin15第二支腳
SoftwareSerial seG3(14,16);//G3用Pin14第一隻腳
//int setPin = 3;//G3開關

dht DHT;   

long pmcf10=0;
long pmcf25=0;
long pmcf100=0;
long pmat10=0;
long pmat25=0;
long pmat100=0;

char buf[50];
int cc=0;//counter

void setup() {
  seG3.begin(9600);//G3
  Serial.begin(9600);//Debug訊息
  Serial1.begin(57600);//給MPU訊息
  Serial.println("Start LASS");
  //pinMode(setPin, OUTPUT); 
  //digitalWrite(setPin, HIGH);
  delay(300);
}

void loop() {
  int count = 0;
  unsigned char c;
  unsigned char high;
  Serial.print(cc);//debug count
  Serial.println(":");
  cc++;
  //取得G3資料
  while (seG3.available()) {
    c = seG3.read();
    if((count==0 && c!=0x42) || (count==1 && c!=0x4d)){
      Serial.println("check failed");
      break;
    }
    if(count > 15){
      Serial.println("complete");
      break;
    }
    else if(count == 4 || count == 6 || count == 8 || count == 10 || count == 12 || count == 14) {
      high = c;
    }
    else if(count == 5){
      pmcf10 = 256*high + c;
      Serial.print("CF=1, PM1.0=");
      Serial.print(pmcf10);
      Serial.println(" ug/m3");
    }
    else if(count == 7){
      pmcf25 = 256*high + c;
      Serial.print("CF=1, PM2.5=");
      Serial.print(pmcf25);
      Serial.println(" ug/m3");
    }
    else if(count == 9){
      pmcf100 = 256*high + c;
      Serial.print("CF=1, PM10=");
      Serial.print(pmcf100);
      Serial.println(" ug/m3");
    }
    else if(count == 11){
      pmat10 = 256*high + c;
      Serial.print("atmosphere, PM1.0=");
      Serial.print(pmat10);
      Serial.println(" ug/m3");
    }
    else if(count == 13){
      pmat25 = 256*high + c;
      Serial.print("atmosphere, PM2.5=");
      Serial.print(pmat25);
      Serial.println(" ug/m3");
    }
    else if(count == 15){
      pmat100 = 256*high + c;
      Serial.print("atmosphere, PM10=");
      Serial.print(pmat100);
      Serial.println(" ug/m3");
    }
    count++;
  }
  while(seG3.available()) seG3.read();
  //取得溫濕度
  DHT.read11(dht_dpin);   //去library裡面找DHT.read11  
  Serial.print("Humidity = ");   
  Serial.print(DHT.humidity);   
  Serial.print("% ");   
  Serial.print("temperature = ");   
  Serial.print(DHT.temperature);   
  Serial.println("C ");   

  //將溫濕度與PM2.5資訊傳給MPU
  Serial1.print("|s_t0=");//Temperature=
  Serial1.print(DHT.temperature);  
  Serial1.print("|s_h0=");//Humi=
  Serial1.print(DHT.humidity);  
  Serial1.print("|s_d0=");//PM2.5=
  Serial1.print(pmat25);
  Serial1.print("|s_d1=");//PM10=
  Serial1.println(pmat100);
  //間隔十秒
  delay(10000);
}
