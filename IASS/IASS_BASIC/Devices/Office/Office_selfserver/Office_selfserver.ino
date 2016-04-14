#include <SPI.h> 
#include <Ethernet.h> 

//====== Configuration ==================================================================

//Sensor代碼定義： 1->溫度, 2->溼度, 3->漏水, 4->煙霧可燃氣體 5->CO, 6->天然氣, 7->PM1,8->PM2.5, 9->PM10, 10->CO2, 11->環境光照度
const unsigned sensorsList[] = {1,2,8,10,11};   //要啟用那些感應器? 建議上限最多5個.
const unsigned nums = 5; //啟用的感應器數量, 最多5個

#define LCD_DISPLAY 1  //是否要啟用LCD顯示?

int unsigned countsAVG = 6;  //要取幾次的sensor值, 去除最大與最小值後, 作最終平均? 最少需3次

const long updateInterval = 2 * 60000;  //單位ms
const String sensorID = "OR";  //給這個感測裝置取個英文ID，注意此ID不要與其它裝置重複.

#define uploadDataServer 1  //要上傳指定的server? 是:1 , 否:0 
const String deviceName = "新竹Office";  //說明此感測裝置用途或地點.
IPAddress DataServerIP(172,30,16,135);  //將資料以GET方式傳到特定server IP, 注意IP間的小數點要改用逗號
const short DataServerPort = 80;  //將資料以GET方式傳到特定 server 的 port 埠號
const String DataServerPage = "/sensors/newdata.php";  //傳到特定server的接收網頁

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

#if defined(LCD_DISPLAY)
  #include <Wire.h> 
  #include <LiquidCrystal_I2C.h>
  LiquidCrystal_I2C lcd(0x27,16,2);  //set the LCD address to 0x27 for a 16 chars and 2 line display
#endif

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
    Serial.println(); 
  }
}

class DataUploader {
  boolean lastConnected;  
    
  public:
    DataUploader() {
      lastConnected = false;
    }

    void uploadDB(String sensorType, String dataUpload) { 
      client.stop(); 
    
      if (client.connect(DataServerIP, 80)) { 
        //Serial.print("GET "+DataServerPage); Serial.print("?sensor="); Serial.print(sensorID); Serial.print(sensorType); Serial.print("&device=");
        //Serial.print(deviceName); Serial.print("&type="); Serial.print(sensorType); Serial.print("&vdata="); Serial.print(dataUpload); Serial.println(" HTTP/1.1"); 
        //client.print("GET "+DataServerPage); client.print("?sensor="); client.print(sensorID); client.print(sensorType); client.print("&device=");
        //client.print(deviceName); client.print("&type="); client.print(sensorType); client.print("&vdata="); client.print(dataUpload); client.println(" HTTP/1.1"); 
        
        Serial.println("GET "+DataServerPage+"?sensor="+sensorID+"&device="+deviceName+"&type="+sensorType+"&vdata="+dataUpload+" HTTP/1.1");                
        client.println("GET "+DataServerPage+"?sensor="+sensorID+"&device="+deviceName+"&type="+sensorType+"&vdata="+dataUpload+" HTTP/1.1"); 
        client.println("Host: "+sensorID); 
        client.println("User-Agent: arduino-ethernet"); 
        client.println("Connection: close"); 
        client.println(); 
        
      } 
      else { 
        Serial.println("connection failed"); 
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
  devicePMS3003 sensorPMS3003;
  deviceMHZ16 sensorMHZ16;
  deviceLight sensorLight;
  DataUploader objDataUpload;
  
  if(LCD_DISPLAY==1) {
    lcd.scrollDisplayLeft();
    delay(750);
  }

  if(lastConnectionTime==0 || (millis() - lastConnectionTime > updateInterval)) {
    String stringUpload;
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
          case 7:
            sensorRead = sensorPMS3003.readValue(10); 
            break;
          case 8:
            sensorRead = sensorPMS3003.readValue(25); 
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

      if(uploadDataServer==1) objDataUpload.uploadDB(String(sensorsList[numSensor]), String(valueTotal));

      
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
            case 7:      
              lcdLine1.concat(padString("PM1", ' ', 7, "center"));
              lcdLine2.concat(padString(String(valueTotal)+"ug", ' ', 7, "center"));
              break;
            case 8:
              lcdLine1.concat(padString("PM2.5", ' ', 7, "center"));
              lcdLine2.concat(padString(String(valueTotal)+"ug", ' ', 7, "center"));
              break;
            case 9:        
              lcdLine1.concat(padString("PM10", ' ', 7, "center"));
              lcdLine2.concat(padString(String(valueTotal)+"ug", ' ', 7, "center"));
              break;
            case 10:       
              lcdLine1.concat(padString("CO2", ' ', 8, "center"));
              lcdLine2.concat(padString(String(valueTotal)+"ppm", ' ', 8, "center"));
              break;
            case 11: 
              lcdLine1.concat(padString("Light", ' ', 8, "center"));
              lcdLine2.concat(padString(String(valueTotal)+"Lux", ' ', 8, "center"));
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
    lastConnectionTime = millis();

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

