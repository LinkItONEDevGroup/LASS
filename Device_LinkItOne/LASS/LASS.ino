

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
                  SOUND_SENSOR_PIN = A1
                  DUST_SENSOR_PIN = 8,	
                  UV_SENSOR_PIN = A0
        LinkItONE:
          Used wifi, gps, flash. Remind wifi/gps antenna needed.
        
        Original:
          The idea come from here: http://iot-hackseries.s3-website-us-west-2.amazonaws.com/linkitone-setup.html

        Optional sample sensor:
          Dust sensor: http://www.seeedstudio.com/depot/Grove-Dust-Sensor-p-1050.html
          UV sensor: http://www.seeedstudio.com/depot/Grove-UV-Sensor-p-1540.html
          Sound sensor: http://www.seeedstudio.com/depot/Grove-Sound-Sensor-p-752.html
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

#define VERSION "V0.4"

#define POLICY_ONLINE_ALWAYS 1
#define POLICY_ONLINE_LESS 2
#define POLICY_ONLINE_DEFAULT 0

#define POLICY_POWER_DONTCARE 0
#define POLICY_POWER_SAVE 1
#define POLICY_POWER_AUTO 2

#define ALARM_MODE_DISABLE 1
#define ALARM_MODE_ENABLE 0

#define LED_MODE_DEFAULT 0 // to show system status and behavior
#define LED_MODE_OFF 1 // To not disturbe the environment, never have LED on 


#define PERIOD_SENSING_IDX 0
#define PERIOD_UPLOAD_IDX 1
#define PERIOD_WIFICHECK_IDX 2

//----- USER CONFIG -----
#define POLICY_ONLINE POLICY_ONLINE_ALWAYS //1: POLICY_ONLINE_ALWAYS 2: POLICY_ONLINE_LESS
                                            
#define POLICY_POWER  POLICY_POWER_DONTCARE //2: POLICY_POWER_AUTO(Auto power saving mode) 0: POLICY_POWER_DONTCARE 1: POLICY_POWER_SAVE
                                            // policy auto check if not charging and battery lower than seting of battery level, switch to power saving mode.
//#define ALARM_MODE ALARM_MODE_DISABLE //0: ALARM_MODE_ENABLE 1: ALARM_MODE_DISABLE

#define POWER_POLICY_BATTERY_LEVEL 70 // When battery level lower than this, trigger power saving mode when power policy is AUTO

#define LED_MODE LED_MODE_DEFAULT  

int period_target[2][3]= // First index is POLICY_POLICY[Sensing period],[Upload period],[Wifi check period], unit is second
  {10,0,60, // don't care power
   10,600,300  // power saving
  };


//----- SENSOR CUSTOMIZATION -----

// Sensor README:
//   3 sensor data: 
//      sound value
//      battery level
//      battery is charging: (0) not charging, (1) charging
#define APP_NAME "EXAMPLE_APP" // REPLACE: this is your unique application name 
#define APP_ID 1               // REPLACE: this is your unique application id. > 65536 if you are use for private purpose

#define SENSOR_CNT 5           // REPLACE: the sensors count that publish to server.
enum pinSensorConfig{
  DUST_SENSOR_PIN = 8,	
  SOUND_SENSOR_PIN = A1,
  UV_SENSOR_PIN = A0 
};

//----- DEAFULT PIN DEFINE -----
enum pinConfig{
	ARDUINO_LED_PIN = 13,
        STORAGE_CHIP_SELECT_PIN = 10
};
//----- GENERAL -----
#define LED_BLINK_DELAY 200

enum info_type{
    INFO_GPS=1,
    INFO_MQTT=2,
    INFO_LOGFILE=3
};

#define DELAY_SYS_EARLY_WAKEUP_MS 11

int current_power_policy=0;

unsigned long currentTime = 0;  // current loop time
unsigned long LastPostTime = 0; // last upload time
unsigned long lastWifiReadyTime = 0; // last wifi ready time


#define SERIAL_BAUDRATE 115200

//----- GPS -----
gpsSentenceInfoStruct info;
char buff[512];
char utcstr[32];
char datestr[32];
#define GPS_SIGNAL_NOCHECK 1   // 0: log or send only when GPS have signal, 1: always log and send even when GPS have no signal

//----- WIFI -----
#define WIFI_SSID "YourSSID"         //  REPLACE: your network SSID (name)
#define WIFI_PASS "YourPASS"         //  REPLACE: your network password (use for WPA, or use as key for WEP)
#define WIFI_AUTH LWIFI_WPA // choose from LWIFI_OPEN, LWIFI_WPA, or LWIFI_WEP.
LWiFiClient wifiClient;

//----- MQTT -----
#define MQTT_PROXY_IP "gpssensor.ddns.net"  // Current LASD server
#define DEVICE_TYPE  "LinkItONE"
#define DEVICE_ID "LASS-Example"    // REPLACE: The device ID you like, please start from LASD. Without this prefix, maybe it will be filter out.
#define MQTT_TOPIC_PREFIX "LASS/Test" 
char mqttTopic[64];

PubSubClient mqttClient((char*)MQTT_PROXY_IP, 1883, msgCallback, wifiClient);
char clientID[50];
char msg[512];

//----- Storage -----
#include <LTask.h>
#include <LFlash.h>
#include <LSD.h>
#include <LStorage.h>


#define Drv LFlash          // use Internal 10M Flash
// #define Drv LSD           // use SD card
//#define STORAGE_CHIP_SELECT_PIN 10
#define LOG_FILENAME "data.log"
String dataString = "";
int logRecordCnt=0;
int logChecked=0; // does log have been checked.

//----- BATTERY -----
#include <LBattery.h>
typedef struct {
  int batteryLevel;
  int charging; 
} BatteryStatus;
BatteryStatus batteryStatus;
//----- SENSORS -----
float sensorValue[SENSOR_CNT];
char sensorUploadString[50]; // Please extend this if you need

//----- STATE -----
int wifi_ready=0;
int gps_ready=0; // 1: when satellites number>1, 0: when satellites==0 

// The logic decide if we should do something
#define LOGIC_WIFI_NEED_CONNECT 1
#define LOGIC_MQTT_NEED_SEND 2
#define LOGIC_DATA_NEED_SAVETOFLASH 3
#define LOGIC_WHAT_LED_STATE 4
#define LOGIC_LOG_NEED_SEND 5

#define LED_STATE_OFF 0
#define LED_STATE_READY 1
#define LED_STATE_ERROR 2

int logic_select(int iWhatLogic){
  int ret = 0;
  switch(iWhatLogic){
    case LOGIC_WIFI_NEED_CONNECT:
      // return: (0) no need to connect, (1) need connect
      // policy_power=don' case: when wifi not ready
      // policy_power=save, last send > upload_period, last_wifi alive > wifi_period 
      // policy_online=POLICY_ONLINE_ALWAYS
      if( wifi_ready==0){
        if(POLICY_ONLINE== POLICY_ONLINE_ALWAYS){
          return 1;
        }
        
        //if(current_power_policy==POLICY_POWER_DONTCARE){
        
        if( (currentTime - lastWifiReadyTime) >(period_target[current_power_policy][PERIOD_WIFICHECK_IDX]*1000)) {
          if( (currentTime - LastPostTime) >(period_target[current_power_policy][PERIOD_UPLOAD_IDX]*1000)) {
            return 1;
          }          
          
        }
      }
      break;
    case LOGIC_MQTT_NEED_SEND:
      // wifi_ready + policy_online=POLICY_ONLINE_ALWAYS
      // wifi_ready + last send > upload_period
      if(wifi_ready && gps_ready) {
        if( POLICY_ONLINE == POLICY_ONLINE_ALWAYS ){
            return 1;
        }
        if ((currentTime - LastPostTime) > (period_target[current_power_policy][PERIOD_UPLOAD_IDX]*1000)) { // all information will show to console, but send out period must > Upload period

          return 1;
    
        }
      }
      
      break;
    case LOGIC_DATA_NEED_SAVETOFLASH:
      //wifi off + gps_ready 
      if((!wifi_ready) && gps_ready){ // only log when wifi fail
        return 1;
      }
      break;
    case LOGIC_WHAT_LED_STATE:
      // LED_MODE=LED_MODE_OFF , led off
      if(LED_MODE==LED_MODE_OFF) return LED_STATE_OFF;
      
      if( wifi_ready & gps_ready ) return LED_STATE_READY;
      else return LED_STATE_ERROR;
      break;
    
    case LOGIC_LOG_NEED_SEND:
      if( wifi_ready){
        if(logChecked==0){ // no matter we save to log in this power up, we need to check log
          logChecked=1;
          return 1;
        }
        if(logRecordCnt > 0){
          return 1;
        }
      }
      break;
  }
  return ret;
}

//----- SENSOR CUSTOMIZATION start
// UV sensor data
float ii; //illumination intensity
int sensor_setup(){
  // Sensor
  pinMode(SOUND_SENSOR_PIN, INPUT); 
  pinMode(DUST_SENSOR_PIN, INPUT);

}

// modify the pulseIn, not wait start state match. because dust sensor may have low for 15s
// in order to speed up the measurement
uint32_t pulseInNoWaitStart( uint32_t pin, uint32_t state, uint32_t timeout)
{
    uint32_t init_time = micros();
	uint32_t curr_time = init_time;
	uint32_t max_time = init_time + timeout;
	int pin_state = 0;

    /* read GPIO info */
    pin_state = digitalRead(pin);
    
    
    // wait for any previous pulse to end
    if ((pin_state == state) && (curr_time < max_time))
    {
        curr_time = micros();
        init_time = curr_time;
    }
	
    // wait for the pulse to start
    while ((pin_state != state) && (curr_time < max_time))
    {
        curr_time = micros();
        init_time = curr_time;
        pin_state = digitalRead(pin);
    }
	
    // wait for the pulse to stop
    while ((pin_state == state) && (curr_time < max_time))
    {
        curr_time = micros();
        pin_state = digitalRead(pin);
    }

    if (curr_time < max_time)
    {
      //Serial.print("Not TIMEOUT=");
      //Serial.println(curr_time - init_time);
        return (curr_time - init_time);
    }
    else
    {
      //Serial.println("TIMEOUT\n");
        return 0;
    }
}


unsigned long duration;
unsigned long dust_starttime=0;
unsigned long dust_sampletime_ms = 30000;//sampe 30s ;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0;

// dust sensor need keep monitor the PWM. This function need dust_sampletime_ms to run, and will block others function
int get_sensor_data_dust(){
  dust_starttime = millis();
  while(1){
    
    duration = pulseInNoWaitStart(DUST_SENSOR_PIN, LOW, dust_sampletime_ms*100);
    //duration = pulseIn(DUST_SENSOR_PIN, LOW);
    lowpulseoccupancy = lowpulseoccupancy+duration;
  
    if ((millis()-dust_starttime) > dust_sampletime_ms)//if the sampel time == 30s
    {
      ratio = lowpulseoccupancy/(dust_sampletime_ms*10.0);  // Integer percentage 0=>100
      concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62; // using spec sheet curve
      Serial.print("concentration = ");
      Serial.print(concentration);
      Serial.println(" pcs/0.01cf");
      Serial.print("ratio = ");
      Serial.println(ratio);
      Serial.print("duration = ");
      Serial.println(duration);
      Serial.println("\n");
      lowpulseoccupancy = 0;
      dust_starttime = millis();
      break; 
     
    }
  }

}
// get UV sensor data
int get_sensor_data_uv(){
  int sensorValue;
  long  sum=0;
  for(int i=0;i<128;i++)
   {  
      sensorValue=analogRead(UV_SENSOR_PIN);
      sum=sensorValue+sum;
      
      delay(2);
   }   
   sum = sum >> 7;
   Serial.print("UV raw value=");
   Serial.println(sum);
   Serial.println("-----UV sensor data-----");
   Serial.print("voltage value:");
   float mv=sum*4980.0/1023.0;
   Serial.print(mv);
   Serial.println("mV");
   ii = mv/0.307;
   Serial.print("illumination intensity=");
   Serial.print(ii);
   Serial.println("(mW/m2)");
   Serial.print("UV Index=");
   Serial.println(ii/200);
  
   delay(20);
   Serial.print("\n");
}
// please customize the how to get the sensor data and store to sensorValue[]
int get_sensor_data(){
  
  if( APP_ID == 1){
    sensorValue[0] = analogRead(SOUND_SENSOR_PIN);//use A1 to read the electrical signal
    Serial.print("SensorValue(Sound):");
    Serial.println(sensorValue[0]);

    getBatteryStatus();
    sensorValue[1] = batteryStatus.batteryLevel;//use A1 to read the electrical signal
    Serial.print("SensorValue(BatteryLevel):");
    Serial.println(sensorValue[1]);

    sensorValue[2] = batteryStatus.charging;//use A1 to read the electrical signal
    Serial.print("SensorValue(BatteryCharging):");
    Serial.println(sensorValue[2]);
    
    
    get_sensor_data_uv();
    sensorValue[3] = ii;
    
    Serial.print("SensorValue(UV sensor):");
    Serial.println(sensorValue[3]); 
    
    get_sensor_data_dust();
    Serial.print("SensorValue(dust sensor):");
    sensorValue[4]=concentration;
    Serial.println(sensorValue[4]); 
    
    
  }
  String msg_sensor = "|values=";
  int i;
  for(i=0;i<SENSOR_CNT;i++)
  {
    if(i>0){
       msg_sensor.concat(",");
    }
    msg_sensor.concat(sensorValue[i]);  
  }
  msg_sensor.toCharArray(sensorUploadString, msg_sensor.length()+1);
}
//----- SENSOR CUSTOMIZATION end

//----- functions that may not need to change -----

//----- GENERAL -----
// setup current policy when POLICY_POWER==AUTO
// Trigger when battery not charging + batteryLevel < POWER_POLICY_BATTERY_LEVEL
void adjustCurrentPowerPolicy(){
  if(POLICY_POWER==POLICY_POWER_AUTO){
    if(batteryStatus.charging==0 && batteryStatus.batteryLevel<POWER_POLICY_BATTERY_LEVEL){
        current_power_policy=POLICY_POWER_SAVE;
    }else{
      current_power_policy=POLICY_POWER_DONTCARE;
    }
  }
}
// pack different type's information, print it for debug
void packInfo(int infoType){
  String msg_tmp;

  switch(infoType){
    case INFO_GPS:
      Serial.print("GPS raw data(GPRMC):");
      Serial.println((char*)info.GPRMC);
      Serial.print("GPS raw data(GPGGA):");
      Serial.println((char*)info.GPGGA);

      
      parseGPGGA((const char*)info.GPGGA);
      parseGPRMC((const char*)info.GPRMC);    
        break;
  
    case INFO_MQTT:
      // mqtt library limit the packet size = 200
      msg_tmp = "|app=";
      msg_tmp.concat(APP_NAME);
      msg_tmp.concat("|device_id=");
      msg_tmp.concat(clientID);
      msg_tmp.concat("|tick=");
      msg_tmp.concat(currentTime);
      msg_tmp.concat("|date=");
      msg_tmp.concat(datestr);
      msg_tmp.concat("|time=");
      msg_tmp.concat(utcstr);      
      msg_tmp.concat("|device=");
      msg_tmp.concat(DEVICE_TYPE);
      msg_tmp.concat(sensorUploadString);
      // default gps result, must have.
      msg_tmp.concat("|gps=");
      msg_tmp.concat((char*)info.GPGGA);
      //msg_tmp.concat("|gpgga=");
      //msg_tmp.concat((char*)info.GPGGA);
      //msg_tmp.concat("|gpgsa=");
      //msg_tmp.concat((char*)info.GPGSA);
      //msg_tmp.concat("|gprmc=");
      //msg_tmp.concat((char*)info.GPRMC);
      //msg_tmp.concat("|gpvtg=");
      //msg_tmp.concat((char*)info.GPVTG);
      //msg_tmp.concat("|gpgsv=");
      //msg_tmp.concat((char*)info.GPGSV);
      //msg_tmp.concat("|glgsv=");
      //msg_tmp.concat((char*)info.GLGSV);
      //msg_tmp.concat("|glgsa=");
      //msg_tmp.concat((char*)info.GLGSA);
      //msg_tmp.concat("|bdgsv=");
      //msg_tmp.concat((char*)info.BDGSV);
      //msg_tmp.concat("|bdgsa=");
      //msg_tmp.concat((char*)info.BDGSA);
      
      
      
      
      msg_tmp.toCharArray(msg, msg_tmp.length()); // the last char will be NULL, design to replace \n
      Serial.print("Pack MQTT Topic:");
      Serial.println(mqttTopic);
      Serial.println(msg);
    
        break; 
     case INFO_LOGFILE:
      // DATA format: @msg
      // Currently, only support one topic in the log
      dataString="";
      dataString.concat("@");
      dataString.concat(msg);
      // print to the serial port too:
      Serial.println("Pack Offline log:");
      Serial.println(dataString);
     
       break;
  }
}
// if data logged, send it out and delete it.
void logSend(){
  int dotCnt=0;
  //Drv.remove((char*)LOG_FILENAME); // for debug
  // upload log only when wifi ready
    if(Drv.exists((char*)LOG_FILENAME)){
  
      Serial.println("Connecting to MQTT Proxy");
      mqttClient.connect(clientID);
      // if log exist, send
      
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
                  mqttClient.publish(mqttTopic, msg);
                  Serial.print(".");
                  dotCnt++;
                  if((dotCnt % 60) ==0){
                    Serial.println(".");
                  }
                  
                }
                Record="";
              }
              else Record.concat(c);            
              //Serial.write(c);
              //Serial.write("!");
          }
          
          // close the file:
          myFile.close();
        
          Drv.remove((char*)LOG_FILENAME);
          Serial.println("\nUpload complete, log file removed");
          logRecordCnt=0;
      }
    }

}

// handler after wifi connected, should be called that wifi may just connected
// ex: send log data
void wifiConnected(){ 
    // Log send
    if(logic_select(LOGIC_LOG_NEED_SEND)){
      logSend();
    }  
  
}
// connecting wifi
void wifiConnecting(){
    // Wifi
    Serial.print("Connecting to Wifi, SSID: ");
    Serial.println(WIFI_SSID);
    // Connect to WiFi (max retry 1 times )

    wifi_ready = Mtk_Wifi_Setup_TryCnt(WIFI_SSID, WIFI_PASS,1);
    if(wifi_ready) {
      printWifiStatus();
      Serial.println("Wifi ready!");
      digitalWrite(ARDUINO_LED_PIN,HIGH);
    }else{
      digitalWrite(ARDUINO_LED_PIN,LOW);
      Serial.println("Wifi not ready!");
    }
}
// light led by different mode and status
void lightLed(){
  if(LED_MODE==LED_MODE_OFF){
    return ;
  }
  if(wifi_ready && gps_ready){
     // blink with default on
     Serial.println("LED:Wifi and gps ready!");
     digitalWrite(ARDUINO_LED_PIN,LOW);
     delay(LED_BLINK_DELAY);
     digitalWrite(ARDUINO_LED_PIN,HIGH);
  }else{
     // blink with default off
     Serial.println("LED:Wifi or gps fail!");
     digitalWrite(ARDUINO_LED_PIN,HIGH);
     delay(LED_BLINK_DELAY);
     digitalWrite(ARDUINO_LED_PIN,LOW);
  }

}
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
    sprintf(utcstr,"%d:%d:%d",hour,minute,second);
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

// currently, this function get data's date information.
void parseGPRMC(const char* GPRMCstr)
{
  /* Refer to http://www.gpsinformation.org/dale/nmea.htm#RMC
  RMC - NMEA has its own version of essential gps pvt (position, velocity, time) data. It is called RMC, The Recommended Minimum, which will look similar to:

  $GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A

  Where:
     RMC          Recommended Minimum sentence C
     123519       Fix taken at 12:35:19 UTC
     A            Status A=active or V=Void.
     4807.038,N   Latitude 48 deg 07.038' N
     01131.000,E  Longitude 11 deg 31.000' E
     022.4        Speed over the ground in knots
     084.4        Track angle in degrees True
     230394       Date - 23rd of March 1994
     003.1,W      Magnetic Variation
     *6A          The checksum data, always begins with *

   */
  int tmp, day, month, year;
  if(GPRMCstr[0] == '$')
  {
    Serial.println("GPS RMC detail result:");
    
    tmp = getComma(9, GPRMCstr);
    day     = (GPRMCstr[tmp + 0] - '0') * 10 + (GPRMCstr[tmp + 1] - '0');
    month   = (GPRMCstr[tmp + 2] - '0') * 10 + (GPRMCstr[tmp + 3] - '0');
    year    = (GPRMCstr[tmp + 4] - '0') * 10 + (GPRMCstr[tmp + 5] - '0');
    
    sprintf(buff, "Date(DD/MM/YY):%d/%d/%d", day, month, year);
    sprintf(datestr,"%d/%d/%d",day,month,year);
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
    //while (!LWiFi.connectWPA(pSSID, pPassword)) {
    while(!LWiFi.connect(pSSID, LWiFiLoginInfo(WIFI_AUTH, pPassword))){
        delay(1000);
        Serial.println("retry WiFi AP");
        i++;
        if(i>=tryCnt) return 0;
    }
    return 1;
}
//----- Battery -----
void getBatteryStatus(){
    batteryStatus.batteryLevel = LBattery.level();
    //sprintf(buff,"battery level = %d", batteryStatus.batteryLevel );
    //Serial.println(buff);

    batteryStatus.charging = LBattery.isCharging();
    //sprintf(buff,"is charging = %d", batteryStatus.charging);
    //Serial.println(buff);

}
// display current setting information to the console
void display_current_setting(){
  String topicTmp="";
  topicTmp.concat(MQTT_TOPIC_PREFIX);
  topicTmp.concat("/");
  topicTmp.concat(APP_NAME);
  topicTmp.toCharArray(mqttTopic, topicTmp.length()+1);
  // General
  Serial.begin(SERIAL_BAUDRATE);
  Serial.print("-------------------- LASS ");
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
  Serial.print(mqttTopic);
  Serial.print(", POLICY_POWER=");
  Serial.print(POLICY_POWER);
  Serial.print(", POLICY_ONLINE=");
  Serial.print(POLICY_ONLINE);
  Serial.print(", PERIOD_SENSING=");
  Serial.print(period_target[current_power_policy][PERIOD_SENSING_IDX]);
  Serial.print(", PERIOD_UPLOAD=");
  Serial.print(period_target[current_power_policy][PERIOD_UPLOAD_IDX]);
  Serial.print(", PERIOD_WIFI=");
  Serial.println(period_target[current_power_policy][PERIOD_WIFICHECK_IDX]); 
}

//----- setup -----
void setup() {
  
  display_current_setting(); 
  
  // General
  String clientIDStr = DEVICE_ID;
  clientIDStr.toCharArray(clientID, clientIDStr.length()+1);
  
  
  if(LED_MODE != LED_MODE_OFF){ // LED_MODE_OFF never light on  
    pinMode(ARDUINO_LED_PIN, OUTPUT);
  }
  
  sensor_setup();
  
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
  
  if(logic_select(LOGIC_WIFI_NEED_CONNECT)){
    wifiConnecting();
    wifiConnected();  
  }
  delay(3000);
  Serial.println("Setup complete! Looping main program");
}

long loop_cnt=0;
//----- loop -----
void loop() {
  currentTime = millis();
  
  Serial.print("-----Loop ID: ");
  Serial.print(loop_cnt);
  loop_cnt++;
  Serial.println(" -----");
  // GPS  
  LGPS.getData(&info);
  packInfo(INFO_GPS);
  
  
  // Sensor
  get_sensor_data();
  
  // MQTT
  
  packInfo(INFO_MQTT);  

  if(logic_select(LOGIC_MQTT_NEED_SEND)){      
      if (!mqttClient.connected()) {
        Serial.println("Reconnecting to MQTT Proxy");
        mqttClient.connect(clientID);
      }
      mqttClient.publish((char*)mqttTopic, msg);
      Serial.println("MQTT sending");
      LastPostTime = currentTime;
      // example:
      // Sensors/DustSensor |device_id=LASD-wuulong|time=20645|device=LinkItONE|values=0|gps=$GPGGA,235959.000,2448.0338,N,12059.5733,E,0,0,,160.1,M,15.0,M,,*4F
      mqttClient.loop();
      if(! mqttClient.connected()){
        wifi_ready=0;
        Serial.println("MQTT send fail!");
      }

  
  }
  
  // Offline log
  packInfo(INFO_LOGFILE);

  if(logic_select(LOGIC_DATA_NEED_SAVETOFLASH)){      
    Serial.println("Saving to file");
    // open the file. note that only one file can be open at a time, so you have to close this one before opening another.
    LFile dataFile = Drv.open(LOG_FILENAME, FILE_WRITE);
  
    // if the file is available, write to it:
    if (dataFile) {
      dataFile.print(dataString); // record not include \n
      dataFile.close();
      logRecordCnt++;
    }
    // if the file isn't open, pop up an error:
    else {
      Serial.println("error opening log file");
    }
  }
  
  // house keeping
  getBatteryStatus();
  
  // change current power policy if setting is AUTO
  adjustCurrentPowerPolicy();

  
  if(logic_select(LOGIC_WIFI_NEED_CONNECT)){
    wifiConnecting();
    wifiConnected(); 
  }

  lightLed();
  //Serial.print(":");
  int usedTime = millis() - currentTime;
  int delayTime = (period_target[current_power_policy][PERIOD_SENSING_IDX]*1000) - usedTime + DELAY_SYS_EARLY_WAKEUP_MS;
  if( delayTime > 0 ){
    delay(delayTime);
  }
}



