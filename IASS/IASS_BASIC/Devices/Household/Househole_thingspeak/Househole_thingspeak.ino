#include <SPI.h> 
#include <Ethernet.h> 

//====== Configuration ==================================================================

//Sensor代碼定義： 1->溫度, 2->溼度, 3->漏水, 5->CO, 6->天然氣, 7->PM1,8->PM2.5, 9->PM10, 10->CO2, , 11->環境光照度
const unsigned sensorsList[] = {1,2,5,6};  //要啟用那些感應器? 建議上限最多5個.
const unsigned nums = 4; //啟用的感應器數量, 最多5個

#define LCD_DISPLAY 1  //是否要啟用LCD顯示?

int unsigned countsAVG = 6;  //要取幾次的sensor值, 去除最大與最小值後, 作最終平均? 最少需3次

const long updateInterval = 2 * 60000;  //單位ms

#define uploadThingsSpeak 1  //要上傳ThingsSpeak? 是:1 , 否:0
const  String writeAPIKey = "YOUR_THINGSPEAK_API_KEY";
const String sensorID = "HR";  //給這個感測裝置取個英文ID，注意此ID不要與其它裝置重複.

byte mac[] = { 0xB8, 0x27, 0xEB, 0xEF, 0x36, 0x31 }; //請自行設定一組MAC Address，可隨意指定，但請勿跟網路上已有的網卡相同避免衝突
const boolean networkDHCP = 1;  //使用DHCP: 1, 使用固定IP: 0

//---> 固定IP,如果使用固定IP資訊請填入下方資訊
IPAddress ip(172,30,17,14);
IPAddress dns_server(172,30,8,6);
IPAddress gateway(172,30,19,253); 
IPAddress subnet(255, 255, 252, 0); 

//====== Ignore here if you don't understand =======================================================

EthernetClient client;  // initialize the library instance
boolean networkStatus = 1;

long lastConnectionTime;

//-------DHT22 ----------------------------------------------------------------
#include "DHT.h"
DHT dht(6, DHT22);

//-------MQ4 -----------------------------------------------------------
#define gasMQ4Pin A2

//-------MQ7 ------------------------------------------------------
#define gasMQ7Pin A3

#if defined(LCD_DISPLAY)
  #include <Wire.h> 
  #include <LiquidCrystal_I2C.h>
  LiquidCrystal_I2C lcd(0x27,16,2);  //set the LCD address to 0x27 for a 16 chars and 2 line display
#endif

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

//-------MQ4 ------------------------------------------------------
class deviceMQ4 {    
  public:
    deviceMQ4() {
      
    }

    float readValue() {
      return analogRead(gasMQ4Pin);
    }
};

//-------MQ7 -----------------------------------------------------------
class deviceMQ7 {    
  public:
    deviceMQ7() {
      
    }

    float readValue() {
      return analogRead(gasMQ7Pin);
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
    
  public:
    DataUploader() {

    }

    void updateThingSpeak(const String tsData) {
      Serial.print("tsData: "); Serial.println(tsData);
      client.stop(); 
    
      if (client.connect("api.thingspeak.com", 80)) { 
        
          //Serial.println("ThingSpeak connected."); 
          // send the HTTP PUT request: 
          client.print("GET /update?key="+writeAPIKey+'&');
          client.print(tsData);
          client.println(" HTTP/1.1"); 
          Serial.print("ThingSpeak: "); Serial.println(tsData);
          client.println("Host: IAAS."+sensorID); 
          client.println("User-Agent: arduino-ethernet"); 
          client.println("Connection: close"); 
          client.println(); 
          lastConnectionTime = millis(); 
        }
        else
        {       
          Serial.println("Connection to ThingSpeak failed.");   
          Serial.println();
        }

        

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
}

void loop() {
  deviceDHT sensorDHT;
  deviceMQ4 sensorMQ4;
  deviceMQ7 sensorMQ7;
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
          case 5:
            sensorRead = sensorMQ4.readValue();  
            break;
          case 6:
            sensorRead = sensorMQ7.readValue();  
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
                lcdLine1.concat(padString("Tem", ' ', 5, "center"));
                lcdLine2.concat(padString(String(valueTotal)+'C', ' ', 5, "center"));
              }
              break;
            case 2:
              if(valueTotal<=100 && valueTotal>=0) {
                lcdLine1.concat(padString("Hum", ' ', 5, "center"));
                lcdLine2.concat(padString(String(valueTotal)+'%', ' ', 5, "center"));
              }
              break;
            case 5:      
              lcdLine1.concat(padString("CO", ' ', 8, "center"));
              lcdLine2.concat(padString(String(valueTotal)+"ppm", ' ', 8, "center"));
              break;
            case 6:
              lcdLine1.concat(padString("Gas", ' ', 8, "center"));
              lcdLine2.concat(padString(String(valueTotal)+"ppm", ' ', 8, "center"));
              break;
            default: 
               // if nothing else matches, do the default
               // default is optional
               break;
        } 
        //Serial.print("Send String: "); Serial.println(stringUpload); 
        //Serial.println(lcdLine1);
        //Serial.println(lcdLine2);
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

