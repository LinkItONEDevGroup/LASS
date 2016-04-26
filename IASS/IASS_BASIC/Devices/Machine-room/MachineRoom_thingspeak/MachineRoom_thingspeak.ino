#include <SPI.h> 
#include <Ethernet.h> 

//====== Configuration ==================================================================

//Sensor代碼定義： 1->溫度, 2->溼度, 3->漏水, 4->煙霧可燃氣體 5->CO, 6->天然氣, 7->PM1,8->PM2.5, 9->PM10, 10->CO2, 11->環境光照度
const unsigned sensorsList[] = {1,2,3,4};   //要啟用那些感應器?
const unsigned nums = 4; //啟用的感應器數量

#define LCD_DISPLAY 1  //是否要啟用LCD顯示?

const String sensorID = "MR";  //給這個感測裝置取個英文ID，注意此ID不要與其它裝置重複.

int unsigned countsAVG = 6;  //要取幾次的sensor值, 去除最大與最小值後, 作最終平均? 最少需3次

#define uploadThingsSpeak 1  //要上傳ThingsSpeak? 是:1 , 否:0
const  String writeAPIKey = "YOUR_THINGSPEAK_API_KEY";

const long updateInterval = 3 * 60000;  //單位ms

byte mac[] = { 0xB8, 0x27, 0xEB, 0xEF, 0x36, 0x30 }; //請自行設定一組MAC Address，可隨意指定，但請勿跟網路上已有的網卡相同避免衝突
const boolean networkDHCP = 1;  //使用DHCP: 1, 使用固定IP: 0

//---> 固定IP,如果使用固定IP資訊請填入下方資訊
IPAddress ip(172,30,17,11);
IPAddress dns_server(172,30,8,6);
IPAddress gateway(172,30,19,253); 
IPAddress subnet(255, 255, 252, 0); 

//====== Ignore here if you don't understand =======================================================

EthernetClient client;  // initialize the library instance
boolean networkStatus = 1;

boolean lastConnected;  
long lastConnectionTime;
int failedCounter;

#if defined(LCD_DISPLAY)
  #include <Wire.h> 
  #include <LiquidCrystal_I2C.h>
  LiquidCrystal_I2C lcd(0x27,16,2);  //set the LCD address to 0x27 for a 16 chars and 2 line display
#endif

//-------DHT22 ----------------------------------------------------------------
#include "DHT.h"
DHT dht(6, DHT22);

//-------Water FC-37 -----------------------------------------------------------
#define waterPin A3

//-------GAS MQ2 ------------------------------------------------------
#define gasMQ2Pin A2

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
      Ethernet.begin(mac, ip, dns, gateway, subnet);
  }

  Serial.print("This is "+sensorID+", IP address: ");  // print your local IP address: 
  if(networkStatus==1) {
    for (byte thisByte = 0; thisByte < 4; thisByte++) { 
      // print the value of each byte of the IP address: 
      Serial.print(Ethernet.localIP()[thisByte], DEC);     
      Serial.print("."); 
    } 
    Serial.println(); 
  }
}

class DataUploader {
  boolean lastConnected;  
    
  public:
    DataUploader() {
      lastConnected = false;
    }

    void updateThingSpeak(const String tsData) {
      //Serial.print("tsData: "); Serial.println(tsData);
      client.stop(); 
    
      if (client.connect("api.thingspeak.com", 80)) { 
        
          //Serial.println("ThingSpeak connected."); 
          // send the HTTP PUT request: 
          client.print("GET /update?key="+writeAPIKey+'&');
          client.print(tsData);
          client.println(" HTTP/1.1"); 
          //Serial.print("ThingSpeak: "); Serial.println(tsData);
          client.println("Host: IAAS."+sensorID); 
          client.println("User-Agent: arduino-ethernet"); 
          client.println("Connection: close"); 
          client.println(); 

        }
        else
        {       
          Serial.println("Connection to ThingSpeak failed.");   
          Serial.println();
        }

        lastConnectionTime = millis(); 

    }
};

// --------- Configuration -----------------------------------------------------


void setup() {
  Serial.begin(9600);

   // --------------> START the Ethernet DHCP connection <--------------------- 
  startEthernet();
  
  if(countsAVG<3) countsAVG = 3;

  if(LCD_DISPLAY==1) {
    lcd.begin();
    lcd.backlight();
    displayLCD(0, "[IAAS]");
    displayLCD(1, "Device "+sensorID);
  }
 
  Serial.println("Device is ready");   

  delay(2500);
}

void loop() {
  deviceDHT sensorDHT;
  deviceFC37 sensorFC37;
  deviceMQ2 sensorMQ2;
  DataUploader objDataUpload;

  if(LCD_DISPLAY==1) {
    lcd.scrollDisplayLeft();
    delay(750);
  }
  
  if(lastConnectionTime==0 || (millis() - lastConnectionTime > updateInterval)) {
    String stringUpload = "";
    String lcdLine1 = "";
    String lcdLine2 = "";
    
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
  
      if(uploadThingsSpeak==1) {
        stringUpload.concat("field" + String(numSensor+1) + '=' + String(valueTotal));
        //Serial.print("     --> stringUpload="); Serial.println(stringUpload);
        if(numSensor<(nums-1)) stringUpload.concat('&');  
        //Serial.print("stringUpload="); Serial.println(stringUpload);
      }

      if(LCD_DISPLAY==1) {
  
        switch (sensorsList[numSensor]) {
            case 1:
              if(valueTotal<=100 && valueTotal>=0) {
                lcdLine1.concat(padString("Tem.", ' ', 5, "center"));
                lcdLine2.concat(padString(String(valueTotal)+'C', ' ', 5, "center"));
              }                   
              break;
            case 2:
              if(valueTotal<=100 && valueTotal>=0) {
                lcdLine1.concat(padString("Hum.", ' ', 5, "center"));
                lcdLine2.concat(padString(String(valueTotal)+'%', ' ', 5, "center"));
              }         
              break;
            case 3:      
              lcdLine1.concat(padString("Water", ' ', 7, "center"));
              lcdLine2.concat(padString(String(valueTotal)+'%', ' ', 7, "center"));
              break;
            case 6:
              lcdLine1.concat(padString("Smoke", ' ', 8, "center"));
              lcdLine2.concat(padString(String(valueTotal)+"ppm", ' ', 7, "center"));
              break;
            default: 
               // if nothing else matches, do the default
               // default is optional
               break;
        }  
      }
      
    }
    
      
    // Update ThingSpeak
    if(uploadThingsSpeak==1) objDataUpload.updateThingSpeak(stringUpload);


    if(LCD_DISPLAY==1) {
      Serial.print("LCD Line1 --> "); Serial.println(lcdLine1);
      Serial.print("LCD Line2 --> "); Serial.println(lcdLine2);
      lcd.clear();
      displayLCD(0, lcdLine1);
      displayLCD(1, lcdLine2);
    }

  }
}

void displayLCD(int yPosition, String txtMSG) {
  int xPos;
   if(txtMSG.length()>16) {
     xPos = 0;
     if(xPos<0) xPos=0;    
   }else{
      xPos = (16-txtMSG.length())/2;
   }
   lcd.setCursor(xPos,yPosition);
   lcd.print(txtMSG);
}

String padString(String stringTXT, char padTXT, int limitWidth,String alignType) {
  int numEmpty = limitWidth - stringTXT.length();
  for (int numI = 0; numI < numEmpty; numI++) {
    if(alignType=="left") {
      stringTXT = stringTXT + padTXT;
    }else if(alignType=="right") {
      stringTXT = padTXT + stringTXT;
    }else{
      (numI % 2 == 0) ? (stringTXT = padTXT + stringTXT) : (stringTXT = stringTXT + padTXT);
    }
  }
  return stringTXT;
}
