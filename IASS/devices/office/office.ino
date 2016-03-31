#include <SPI.h> 
#include <Ethernet.h> 

//====== Configuration ==================================================================

//Sensor代碼定義： 1->溫度, 2->溼度, 3->漏水, 5->CO, 6->天然氣, 7->PM2.5,8->PM10, 9->PM100, 10->CO2, , 11->環境光照度
const unsigned sensorsList[] = {1,2,7,8,9,10,11};   //要啟用那些感應器?
const unsigned nums = 7; //啟用的感應器數量

const String sensorID = "MR";  //給這個感測裝置取個英文ID，注意此ID不要與其它裝置重複.
const String deviceName = "電腦機房";  //說明此感測裝置用途或地點.
int unsigned countsAVG = 6;  //要取幾次的sensor值, 去除最大與最小值後, 作最終平均? 最少需3次

const boolean uploadThingsSpeak = 1;  //要上傳ThingsSpeak? 是:1 , 否:0
const  String writeAPIKey = "0OEBXQUXVZCVW7Z5";
const long updateThingSpeakInterval = 5 * 60000;  //單位ms

const boolean uploadDataServer = 0;  //要上傳指定的server? 是:1 , 否:0 
IPAddress DataServerIP(192,168,3,2);  //將資料以GET方式傳到特定server IP，若不需要上傳則用 ""
const short DataServerPort = 80;  //將資料以GET方式傳到特定 server 的 port 埠號
const String DataServerPage = "/iaas/update.php";  //將資料以GET方式傳到特定server的接收網頁路徑

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; //請自行設定一組MAC Address，可隨意指定，但請勿跟網路上已有的網卡相同避免衝突
const boolean networkDHCP = 1;  //使用DHCP: 1, 使用固定IP: 0

//---> 固定IP,如果使用固定IP資訊請填入下方資訊
byte ip[] = { 10,0,0,35 };    //如果沒有成功得到DHCP的位址, 則會使用這個IP.
byte myDns[] = { 8,8,8,8 };    //如果沒有成功得到來自DHCP的DNS, 則會使用這個DNS IP.
byte gateway[] = { 10,0,0,254 };
byte subnet[] = { 255, 255, 255, 0 };

//====== Ignore here if you don't understand =======================================================

EthernetClient client;  // initialize the library instance
boolean networkStatus = 1;

boolean lastConnected;  
long lastConnectionTime;
int failedCounter;
//-------DHT22 ----------------------------------------------------------------
#include "DHT.h"
DHT dht(6, DHT22);
//-------CO2 MH-Z16 -----------------------------------------------------------
#include <SoftwareSerial.h>
SoftwareSerial sensor(7,8);  //pinTx, pinRx
//-------PM2.5 G3 PM3003 ------------------------------------------------------
#include "serialReadPMValue.h"
//-------Light TEMT6000 -------------------------------------------------------
#define lightPin A1

//====== Sensor Classes =======================================================
class deviceLight {    
  public:
    deviceLight() {
      dht.begin();
    }

    float readValue() {
      float volts = analogRead(lightPin) * 5.0 / 1024.0;
      float amps = volts / 10000.0;  // across 10,000 Ohms
      float microamps = amps * 1000000;
      return microamps * 2.0;  // lux value
    }
};

class deviceDHT {    
  public:
    deviceDHT() {
      
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

//-------PM2.5 G3 PM3003 ------------------------------------------------------
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

//-------CO2 MH-Z16 -----------------------------------------------------------
class deviceMHZ16 {  
  #define DEBUG 0
  unsigned char dataRevice[9];
  int CO2PPM;
  int temperature; 
  const unsigned char cmd_get_sensor[9] =
      {
          0xff, 0x01, 0x86, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x79
      };
  
  public:
    deviceMHZ16() {
      sensor.begin(9600); 
      
    }

    long readValue() {
        while(!dataCO2Recieve())
        {
            delay(500);
        }
        return CO2PPM;
    }

    bool dataCO2Recieve(void)
    {
        byte data[9];
        int i = 0;
     
        //transmit command data
        for(i=0; i<sizeof(cmd_get_sensor); i++)
        {
            sensor.write(cmd_get_sensor[i]);
        }
        delay(10);
        //begin reveiceing data
        if(sensor.available())
        {
            while(sensor.available())
            {
                for(int i=0;i<9; i++)
                {
                    data[i] = sensor.read();
                }
            }
        }
     
    #if DEBUG
        for(int j=0; j<9; j++)
        {
            Serial.print(data[j]);
            Serial.print(" ");
        }
        Serial.println("");
    #endif
     
        if((i != 9) || (1 + (0xFF ^ (byte)(data[1] + data[2] + data[3]
        + data[4] + data[5] + data[6] + data[7]))) != data[8])
        {
            return false;
        }
        CO2PPM = (int)data[2] * 256 + (int)data[3];
        temperature = (int)data[4] - 40;
     
        return true;
    }
};

void startEthernet() {
  
  client.stop();

  Serial.println("Connecting Arduino to network...");
  Serial.println();  

  delay(1000);

  if(networkDHCP==1) {
    if (Ethernet.begin(mac) == 0) { 
      Serial.println("無法取得DHCP"); 
      networkStatus = 0;
    } 
  }else{
      Ethernet.begin(mac, ip, gateway, subnet);
  }

  Serial.print("This is "+sensorID+", IP address: ");  // print your local IP address: 
  if(networkStatus==1) {
    for (byte thisByte = 0; thisByte < 4; thisByte++) { 
      // print the value of each byte of the IP address: 
      Serial.print(Ethernet.localIP()[thisByte], DEC);     
      Serial.print("."); 
    } 
  }
}

class DataUploader {
  boolean lastConnected;  
  int failedCounter;
    
  public:
    DataUploader() {
      lastConnected = false;
      failedCounter = 0;
    }

    void updateThingSpeak(String tsData) {
      client.stop(); 
    
      if (client.connect("api.thingspeak.com", 80)) { 
          Serial.println("ThingSpeak connected."); 
          // send the HTTP GET request: 
          client.println("GET /update?key="+writeAPIKey+"&"+tsData+" HTTP/1.1"); 
          Serial.print("ThingSpeak: "); Serial.println("/update?key="+writeAPIKey+"&"+tsData);
          client.println("Host: IAAS."+sensorID); 
          client.println("User-Agent: arduino-ethernet"); 
          client.println("Connection: close"); 
          client.println(); 

          failedCounter = 0;
        }
        else
        {
          failedCounter++;          
          Serial.println("Connection to ThingSpeak failed ("+String(failedCounter, DEC)+")");   
          Serial.println();
        }

        lastConnectionTime = millis(); 
        lastConnected = client.connected();
    }

    void updateDataServer(String tsData) {
      client.stop(); 
    
      if (client.connect(DataServerIP, DataServerPort)) { 
          Serial.println("DataServer connected."); 
          // send the HTTP GET request: 
          client.println("GET "+ IpAddress2String(DataServerIP) +"?"+tsData+" HTTP/1.1"); 
          Serial.print("DataServer: "); Serial.println(DataServerPage+"?"+tsData);
          client.println("Host: IAAS."+sensorID); 
          client.println("User-Agent: arduino-ethernet"); 
          client.println("Connection: close"); 
          client.println(); 

          failedCounter = 0;
        }
        else
        {
          failedCounter++;          
          Serial.println("Connection to " + IpAddress2String(DataServerIP) + " failed ("+String(failedCounter, DEC)+")");   
          Serial.println();
        }

        lastConnectionTime = millis(); 
        lastConnected = client.connected();
    }

    String IpAddress2String(const IPAddress& ipAddress) {
      return String(ipAddress[0]) + String(".") +\
      String(ipAddress[1]) + String(".") +\
      String(ipAddress[2]) + String(".") +\
      String(ipAddress[3])  ; 
    }
};

// --------- Configuration -----------------------------------------------------
deviceDHT sensorDHT;
devicePMS3003 sensorPMS3003;
deviceMHZ16 sensorMHZ16;
deviceLight sensorLight;

DataUploader serviceUploadData;

void setup() {
  Serial.begin(9600);

   // --------------> START the Ethernet DHCP connection <--------------------- 
  startEthernet();
  
  if(countsAVG<3) countsAVG = 3;

  Serial.println("Device is ready");   
}

void loop() {

  if(lastConnectionTime==0 || (millis() - lastConnectionTime > updateThingSpeakInterval)) {
    String stringUpload = "";
    for (int numSensor = 0; numSensor < nums; numSensor++){
      int valueTotal = 0; 
      int maxValue = 0;    
      int minValue = 999999;   
      int sensorRead = 0; 
      String sensorValue;
      String sensor_ID = sensorID + "-" + String(sensorsList[numSensor]);
  
      Serial.print("["+sensor_ID+"] "); 
      for (int i=0; i < countsAVG; i++){ 
  
        switch (sensorsList[numSensor]) {
          case 1:
            sensorRead = sensorDHT.readValue('T');     
            break;
          case 2:
            sensorRead = sensorDHT.readValue('H');  
            break;
          case 7:
            sensorRead = sensorPMS3003.readValue(25); 
            break;
          case 8:
            sensorRead = sensorPMS3003.readValue(10); 
            break;
          case 9:
            sensorRead = sensorPMS3003.readValue(100); 
            break;
          case 10:
            sensorRead = sensorMHZ16.readValue(); 
            break;
          case 11:
            sensorRead = sensorLight.readValue(); 
            break;
          default: 
             // if nothing else matches, do the default
             // default is optional
             break;
          }
          if(i==0) maxValue = sensorRead; minValue = sensorRead; 
          valueTotal = valueTotal + sensorRead; 
          if(sensorRead>=maxValue) maxValue=sensorRead; 
          if(sensorRead<=minValue) minValue=sensorRead; 
          Serial.print(sensorRead);
  
          ((i+1) < countsAVG)?Serial.print(","):Serial.print("  ");
      }

      valueTotal = (valueTotal-maxValue-minValue)/(countsAVG-2);       
      Serial.println("field" + String(numSensor+1) + '='+valueTotal);
  
      stringUpload = stringUpload + "field" + String(numSensor+1) + '='+valueTotal;
      if(numSensor<(nums-1)) stringUpload=stringUpload+'&';  
    }
    
      
    // Update ThingSpeak
    if(uploadThingsSpeak==1) serviceUploadData.updateThingSpeak(stringUpload);

    if(uploadDataServer==1) serviceUploadData.updateDataServer(stringUpload);
      
    // Check if Arduino Ethernet needs to be restarted
    if (failedCounter > 3 ) {startEthernet();}

  }
}


