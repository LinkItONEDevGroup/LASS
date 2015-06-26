/*
	Location Aware Sensor System(LASS) by Linkit ONE

        Vision: We hope location aware sensor data can be easy to provide, share, and become useful to everyone.
        
        Sensor Used as example:
          Sound Sensor: http://www.seeedstudio.com/wiki/Grove_-_Sound_Sensor
          
	The circuit:
		Output PIN: 
                  ARDUINO_LED_PIN = 13,
                  STORAGE_CHIP_SELECT_PIN = 10 (Reserved)
                Sensor Input PIN:
                  SOUND_SENSOR_PIN = A0 
        LinkItONE:
          Used wifi, gps, flash. Remind wifi/gps antenna needed.
        
        Original:
          The idea come from here: http://iot-hackseries.s3-website-us-west-2.amazonaws.com/linkitone-setup.html

	Created 26/06/2015
	By Wuulong

	https://github.com/wuulong/LinkitOneGroup

*/

#include <LWiFi.h>
#include <LWiFiClient.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <MtkAWSImplementations.h>
#include <LGPS.h>

#define VERSION "V0.01"
//----- PIN DEFINE -----
enum pinConfig{
	ARDUINO_LED_PIN = 13,
        STORAGE_CHIP_SELECT_PIN = 10
};
//----- GENERAL -----
#define LOOP_DELAY 10000

unsigned long currentTime = 0;
unsigned long LastPostTime = 0;
enum {
  POSTING_PERIOD_MS = 5000
};

//----- GPS -----
gpsSentenceInfoStruct info;
char buff[256];
#define GPS_SIGNAL_NOCHECK 0   // 0: log or send only when GPS have signal, 1: always log and send even when GPS have no signal

//----- WIFI -----
#define WIFI_SSID "YourSSID"         //  REPLACE: your network SSID (name)
#define WIFI_PASS "YourPASS"      //  REPLACE: your network password (use for WPA, or use as key for WEP)
LWiFiClient wifiClient;

//----- MQTT -----
#define MQTT_PROXY_IP "gpssensor.ddns.net"  // Current LASD server
#define DEVICE_TYPE  "LinkItONE"
#define DEVICE_ID "LASD-Example"    // REPLACE: The device ID you like, please start from LASD. Without this prefix, maybe it will be filter out.
#define MQTT_TOPIC "Sensors/SoundSensor"  // REPLACE: If you use different sensors, you may like to change.

PubSubClient mqttClient((char*)MQTT_PROXY_IP, 1883, msgCallback, wifiClient);
char clientID[50];
char msg[180];

//----- Storage -----
#include <LTask.h>
#include <LFlash.h>
#include <LSD.h>
#include <LStorage.h>


#define Drv LFlash          // use Internal 10M Flash
// #define Drv LSD           // use SD card
//#define STORAGE_CHIP_SELECT_PIN 10
#define LOG_FILENAME "data.log"
//----- SENSOR -----
#define SOUND_SENSOR_PIN A1

//----- STATE -----
int wifi_ready=0;
int gps_ready=0; // 1: when satellites number>1, 0: when satellites==0 


//----- setup -----
void setup() {
  // General
  Serial.begin(115200);
  Serial.print("-------------------- LASD ");
  Serial.print(VERSION);
  Serial.println(" -------------------------");
  Serial.println("User configuration");
  Serial.print("SSID=");
  Serial.print(WIFI_SSID);
  Serial.print(", MQTT_IP=");
  Serial.print(MQTT_PROXY_IP);
  Serial.print(", DeviceID=");
  Serial.print(DEVICE_ID);
  Serial.print(", TOPIC=");
  Serial.println(MQTT_TOPIC);
  
  
  pinMode(ARDUINO_LED_PIN, OUTPUT);

  // Sensor
  pinMode(SOUND_SENSOR_PIN, INPUT); 
  
  // GPS
  LGPS.powerOn();
  Serial.println("LGPS Power on"); 

  // Flash
  Serial.print("Initializing flash interface...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(STORAGE_CHIP_SELECT_PIN, OUTPUT);
  
  // see if the card is present and can be initialized:
  Drv.begin();
  Serial.println(", Flash initialized.");
  
  // Wifi
  Serial.print("Connecting to Wifi, SSID: ");
  Serial.println(WIFI_SSID);
  // Connect to WiFi (max retry 3 times )
  wifi_ready = Mtk_Wifi_Setup_TryCnt(WIFI_SSID, WIFI_PASS,3);
  if(wifi_ready) {
    printWifiStatus();
    digitalWrite(ARDUINO_LED_PIN,HIGH);
  }else{
    digitalWrite(ARDUINO_LED_PIN,LOW);
    Serial.println("Wifi not ready!");
  }
  
  // MQTT
  String clientIDStr = DEVICE_ID;
  clientIDStr.toCharArray(clientID, clientIDStr.length()+1);

  // upload log only when wifi ready
  if( wifi_ready ){
    Serial.println("Connecting to MQTT Proxy");
    mqttClient.connect(clientID);
    // if log exist, send
    if(Drv.exists((char*)LOG_FILENAME)){
      
      // re-open the file for reading:
      LFile myFile = Drv.open(LOG_FILENAME);
      if (myFile) {
          Serial.println(LOG_FILENAME);
          myFile.seek(0);
          String Record="@";
          // read from the file until there's nothing else in it:
          while (myFile.available()) {
              char c= myFile.read();
              if(c=='@'){
                if(Record!="@"){
                  Record.toCharArray(msg, Record.length()+1);
                  mqttClient.publish((char*)MQTT_TOPIC, msg);
                  Serial.println("");
                }
                Record="";
              }
              else Record.concat(c);            
              Serial.write(c);
              //Serial.write("!");
          }
          
          // close the file:
          myFile.close();
        
          Drv.remove((char*)LOG_FILENAME);
          Serial.println("Upload complete, log file removed");
      }
    }
  }
 
  Serial.println("Setup complete! Looping main program");
}



//----- loop -----
void loop() {

  // GPS  
  LGPS.getData(&info);
  Serial.print("GPS raw data: ");
  Serial.println((char*)info.GPGGA); 
  parseGPGGA((const char*)info.GPGGA);
  
  // Sensor
  int sensorValue = analogRead(SOUND_SENSOR_PIN);//use A0 to read the electrical signal
  Serial.print("SensorValue:");
  Serial.println(sensorValue);
  
  currentTime = millis();
  
  String accel_msg = "|device_id=";
  accel_msg.concat(clientID);
  accel_msg.concat("|time=");
  accel_msg.concat(currentTime);
  accel_msg.concat("|device=");
  accel_msg.concat(DEVICE_TYPE);
  accel_msg.concat("|values=");
  accel_msg.concat(sensorValue);
  accel_msg.concat("|gps=");
  accel_msg.concat((char*)info.GPGGA);
  
  accel_msg.toCharArray(msg, accel_msg.length()); // the last char will be NULL, design to replace \n
  Serial.print("MQTT Topic:");
  Serial.println((char*)MQTT_TOPIC);
  Serial.println(msg);
      
  if ((currentTime - LastPostTime) > POSTING_PERIOD_MS) { // all information will show to console, but send out period must > POSTING_PERIOD_MS
    if(wifi_ready && gps_ready) {
      if (!mqttClient.connected()) {
        Serial.println("Reconnecting to MQTT Proxy");
        mqttClient.connect(clientID);
      }
      mqttClient.publish((char*)MQTT_TOPIC, msg);
      // example:
      // Sensors/DustSensor |device_id=LASD-wuulong|time=20645|device=LinkItONE|values=0|gps=$GPGGA,235959.000,2448.0338,N,12059.5733,E,0,0,,160.1,M,15.0,M,,*4F

    }
  }
  
  
  // Offline log
  String dataString = "";
  // DATA format: @msg
  // Currently, only support one topic in the log
  dataString.concat("@");
  dataString.concat(msg);
  // print to the serial port too:
  Serial.println("Offline log:");
  Serial.println(dataString);

  if((!wifi_ready) && gps_ready){ // only log when wifi fail

    // open the file. note that only one file can be open at a time, so you have to close this one before opening another.
    LFile dataFile = Drv.open(LOG_FILENAME, FILE_WRITE);
  
    // if the file is available, write to it:
    if (dataFile) {
      dataFile.print(dataString); // record not include \n
      dataFile.close();
    }
    // if the file isn't open, pop up an error:
    else {
      Serial.println("error opening log file");
    }
  }
  
  LastPostTime = currentTime;

  mqttClient.loop();
  
  if(wifi_ready && gps_ready){
     // blink with default on
     Serial.println("Wifi and gps ready!");
     digitalWrite(ARDUINO_LED_PIN,LOW);
     delay(200);
     digitalWrite(ARDUINO_LED_PIN,HIGH);
  }else{
     // blink with default off
     Serial.println("Wifi or gps fail!");
     digitalWrite(ARDUINO_LED_PIN,HIGH);
     delay(200);
     digitalWrite(ARDUINO_LED_PIN,LOW);
  }
  delay(LOOP_DELAY);
}

//----- functions that may not need to change -----

// callback to handle incomming MQTT messages
void msgCallback(char* topic, byte* payload, unsigned int length) { }

//----- GPS -----

static unsigned char getComma(unsigned char num,const char *str)
{
  unsigned char i,j = 0;
  int len=strlen(str);
  for(i = 0;i < len;i ++)
  {
     if(str[i] == ',')
      j++;
     if(j == num)
      return i + 1; 
  }
  return 0; 
}

static double getDoubleNumber(const char *s)
{
  char buf[10];
  unsigned char i;
  double rev;
  
  i=getComma(1, s);
  i = i - 1;
  strncpy(buf, s, i);
  buf[i] = 0;
  rev=atof(buf);
  return rev; 
}

static double getIntNumber(const char *s)
{
  char buf[10];
  unsigned char i;
  double rev;
  
  i=getComma(1, s);
  i = i - 1;
  strncpy(buf, s, i);
  buf[i] = 0;
  rev=atoi(buf);
  return rev; 
}

void parseGPGGA(const char* GPGGAstr)
{
  /* Refer to http://www.gpsinformation.org/dale/nmea.htm#GGA
   * Sample data: $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
   * Where:
   *  GGA          Global Positioning System Fix Data
   *  123519       Fix taken at 12:35:19 UTC
   *  4807.038,N   Latitude 48 deg 07.038' N
   *  01131.000,E  Longitude 11 deg 31.000' E
   *  1            Fix quality: 0 = invalid
   *                            1 = GPS fix (SPS)
   *                            2 = DGPS fix
   *                            3 = PPS fix
   *                            4 = Real Time Kinematic
   *                            5 = Float RTK
   *                            6 = estimated (dead reckoning) (2.3 feature)
   *                            7 = Manual input mode
   *                            8 = Simulation mode
   *  08           Number of satellites being tracked
   *  0.9          Horizontal dilution of position
   *  545.4,M      Altitude, Meters, above mean sea level
   *  46.9,M       Height of geoid (mean sea level) above WGS84
   *                   ellipsoid
   *  (empty field) time in seconds since last DGPS update
   *  (empty field) DGPS station ID number
   *  *47          the checksum data, always begins with *
   */
  double latitude;
  double longitude;
  int tmp, hour, minute, second, num ;
  if(GPGGAstr[0] == '$')
  {
    Serial.println("GPS detail result:");
    
    tmp = getComma(1, GPGGAstr);
    hour     = (GPGGAstr[tmp + 0] - '0') * 10 + (GPGGAstr[tmp + 1] - '0');
    minute   = (GPGGAstr[tmp + 2] - '0') * 10 + (GPGGAstr[tmp + 3] - '0');
    second    = (GPGGAstr[tmp + 4] - '0') * 10 + (GPGGAstr[tmp + 5] - '0');
    
    sprintf(buff, "UTC timer %2d-%2d-%2d", hour, minute, second);
    Serial.println(buff);
    
    tmp = getComma(2, GPGGAstr);
    latitude = getDoubleNumber(&GPGGAstr[tmp]);
    tmp = getComma(4, GPGGAstr);
    longitude = getDoubleNumber(&GPGGAstr[tmp]);
    sprintf(buff, "latitude = %10.4f, longitude = %10.4f", latitude, longitude);
    Serial.println(buff); 
    
    tmp = getComma(7, GPGGAstr);
    num = getIntNumber(&GPGGAstr[tmp]);    
    sprintf(buff, "satellites number = %d", num);
    if(num>0 || GPS_SIGNAL_NOCHECK ) {
      gps_ready=1;
    }else{
      gps_ready=0;
    }
    Serial.println(buff); 
  }
  else
  {
    Serial.println("Not get data"); 
  }
}

// return - 0: retry and timeout, 1: success
int Mtk_Wifi_Setup_TryCnt(const char* pSSID, const char* pPassword, int tryCnt) {
    // attempt to connect to Wifi network:
    LWiFi.begin();
    int i=0;
    while (!LWiFi.connectWPA(pSSID, pPassword)) {
        delay(1000);
        Serial.println("retry WiFi AP");
        i++;
        if(i>=tryCnt) return 0;
    }
    return 1;
}


