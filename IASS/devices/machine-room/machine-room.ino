#include <SPI.h> 
#include <Ethernet.h> 

//====== Configuration ==================================================================

//Sensor代碼定義： 1->溫度, 2->溼度, 3->漏水(回傳值的為漏水機率%), 5->CO or MQ131,135,136,137,138,216 series, 6->天然氣或可燃氣體 or MQ2~9 series, 7->PM2.5,8->PM10, 9->PM100, 10->CO2, , 11->環境光照度
const unsigned sensorsList[] = {1,2,3,6};   //要啟用那些感應器?
const unsigned nums = 4; //啟用的感應器數量

const String sensorID = "MR";  //給這個感測裝置取個英文ID，注意此ID不要與其它裝置重複.
const String deviceName = "電腦機房";  //說明此感測裝置用途或地點.
int unsigned countsAVG = 6;  //要取幾次的sensor值, 去除最大與最小值後, 作最終平均? 最少需3次

const boolean uploadThingsSpeak = 1;  //要上傳ThingsSpeak? 是:1 , 否:0
const  String writeAPIKey = "XK76Q4EWPDK5WY4K";
const long updateThingSpeakInterval = 5 * 60000;  //單位ms

const boolean uploadDataServer = 0;  //要上傳指定的server? 是:1 , 否:0 
IPAddress DataServerIP(192,168,3,2);  //將資料以GET方式傳到特定server IP
const short DataServerPort = 80;  //將資料以GET方式傳到特定 server 的 port 埠號
const String DataServerPage = "/iaas/update.php";  //傳到特定server的接收網頁路徑

byte mac[] = { 0xB8, 0x27, 0xEB, 0xEF, 0x36, 0x30 }; //請自行設定一組MAC Address，可隨意指定，但請勿跟網路上已有的網卡相同避免衝突
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

//-------Water FC-37 -----------------------------------------------------------
#define waterPin A3

//-------GAS MQ2 ------------------------------------------------------
#define gasMQ2Pin A5

//-------Light TEMT6000 -------------------------------------------------------
#define lightPin A1

//====== Sensor Classes =======================================================

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

//-------FC-37 ------------------------------------------------------
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

//-------MQ2 -----------------------------------------------------------
class deviceMQ2 {    
  public:
    deviceMQ2() {      
    }

    float readValue() {
      return analogRead(gasMQ2Pin);
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
deviceFC37 sensorFC37;
deviceMQ2 sensorMQ2;

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
          case 3:
            sensorRead = sensorFC37.readValue();  
            break;
          case 6:
            sensorRead = sensorMQ2.readValue();  
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


