/*
  Location Aware Sensor System(LASS) by Linkit ONE 

        Vision: We hope location aware sensor data can be easy to provide, share, and become useful to everyone.
        
        Sensor Used as example:
          Sound Sensor: http://www.seeedstudio.com/wiki/Grove_-_Sound_Sensor
          
  Board jumper setting:
            SPI/SD = SPI
            Mass Storage Bootup/Normal Bootup = Normal Bootup
            USB Power Source/Li-Battery Power Source = USB Power Source 
        The circuit:
    Default Output PIN: 
                  ARDUINO_LED_PIN = 13,
                  STORAGE_CHIP_SELECT_PIN = 10 (Reserved)
                Sensor Input PIN:
                  SOUND_SENSOR_PIN = A1
                  DUST_SENSOR_PIN = 8,  cal
                  UV_SENSOR_PIN = A0
                  TEMP-HUMID_SENSOR_PIN = 2
                  BAROMETER_SENSOR_PIN -> I2C
                  LIGHT_SENSOR -> I2C
                Action Output PIN:
                  BUZZER_ALARM_PIN = 3
        LinkItONE:
          Used wifi, gps, flash. Remind wifi/gps antenna needed.
        
        Sensor position definition:
        0-9(System sensor):   reserved for system sensor, should not be modified for individual application purpose.
        10-19(User sensor): user customization purpose, user can freely customize this area
        
        Default system sensor order:
          0: record_id, 1: battery level, 2: battery charging, 3: ground speed ( Km/hour ) 
        Default user sensor order:
          10: dust sensor, 11: UV  sensor, 12: sound sensor 
        Default user sensor order in APP-3 (MAPS):
          10: barometer sensor (high accuracy), 11&12: temperature & humidity sensor pro, 13: digital light sensor

  Default Sensor Type: ( -: void;  0-9: reserved for system sensor;  A-Z: LASS default sensors;  a-z: reserved for other sensors)
    -: unused
    0: record ID
    1: battery level
    2: battery charging
    3: ground speed
    4: wifi debug: reconnect count
    d: dust sensor
    u: UV sensor
    s: sound sensor
    b: barometer sensor (hPa)
    t: temperature sensor (degree C)
    h: humidity sensor (%)
    l: light sensor (LUX)
        
        Original:
          The idea come from here: http://iot-hackseries.s3-website-us-west-2.amazonaws.com/linkitone-setup.html

        Optional sample sensor:
          AP1-SENSOR_ID_DUST: Dust sensor: http://www.seeedstudio.com/depot/Grove-Dust-Sensor-p-1050.html
          AP1-SENSOR_ID_UV: UV sensor: http://www.seeedstudio.com/depot/Grove-UV-Sensor-p-1540.html
          AP1-SENSOR_ID_SOUND: Sound sensor: http://www.seeedstudio.com/depot/Grove-Sound-Sensor-p-752.html
          AP2-SENSOR_ID_DUST: Dust sensor http://tw.taobao.com/item/43750623059.htm
          AP3-SENSOR_ID_BAROMETER: Barometer sensor (high accuracy): http://www.seeedstudio.com/wiki/Grove_-_Barometer_(High-Accuracy)
          AP3-SENSOR_ID_TEMPERATURE: Temperture and Humidity sensor pro: http://www.seeedstudio.com/wiki/Grove_-_Temperature_and_Humidity_Sensor_Pro
          AP3-SENSOR_ID_HUMIDITY: Temperture and Humidity sensor pro: http://www.seeedstudio.com/wiki/Grove_-_Temperature_and_Humidity_Sensor_Pro
          AP3-SENSOR_ID_LIGHT: Digital Light sensor: http://www.seeedstudio.com/wiki/Grove_-_Digital_Light_Sensor
          AP4-SENSOR_ID_DUST: Dust sensor http://tw.taobao.com/item/43750623059.htm
          AP4-SENSOR_ID_TEMPERATURE: Temperture and Humidity sensor pro: http://www.seeedstudio.com/wiki/Grove_-_Temperature_and_Humidity_Sensor_Pro
          AP4-SENSOR_ID_HUMIDITY: Temperture and Humidity sensor pro: http://www.seeedstudio.com/wiki/Grove_-_Temperature_and_Humidity_Sensor_Pro
        Optional alarm:
          Buzzer : http://www.seeedstudio.com/depot/Grove-Buzzer-p-768.html
        Created 26/06/2015
  By Wuulong

  https://github.com/wuulong/LinkitOneGroup

*/
//ALL Configuration is at configuration.h
#include "configuration.h"

#define VER_FORMAT "3"  // version number has been increased to 2 since v0.7.0
#define VER_APP "0.8.1"

// Blynk
#if BLYNK_ENABLE == 1
  #define ARDUINO 150 // to avoid Blynk library use yield() in function run(), without this. system will crash!
  #include <BlynkSimpleLinkItONE.h>
  #define BLYNK_POOLING_TIME 1000 // the blynk pooling loop can't delay too long, quick check is 5000 ms, set is < 5000
#else
  #include <LWiFi.h>
  #include <LWiFiClient.h>
#endif

//Time NTP
#include <LWiFiUdp.h>
LWiFiUDP Udp;
unsigned int localPort = 2390;      // local port to listen for UDP packets
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
uint32_t epochSystem = 0; // timestamp of system boot up
IPAddress timeServerIP(129, 6, 15, 28);


//web-server
#include <LWiFiServer.h>
LWiFiServer server(80);

#include <PubSubClient.h>
#include <Wire.h>
#include <MtkAWSImplementations.h>
#include <LGPS.h>
#include <LTask.h> 
#include "vmthread.h" 
#include "stddef.h" 

// The user setting structure
typedef struct{
  // version control
  unsigned int set_version;
  // wifi section
  char wifi_ssid[32];
  char wifi_pass[11];
  unsigned int wifi_auth;
  // mqtt
  char device_type[32];
  char device_id[32];
  char mqtt_topic_prefix[64];
  char partner_id[32];
  //gps
  char gps_fix_info_lat[32];
  char gps_fix_info_lot[32];
  char gps_fix_info_alt[32];
  //other
  char blynk_auth[33];
  //reserved, prepare for some compatibility need
  char reserved[256];
} SETTING;

SETTING setting;

int period_target[2][3]= // First index is POLICY, [Sensing period],[Upload period],[Wifi check period], unit is second
  {
#if ADJ_MODE == ADJ_MODE_FLY    
    5,5,300, // don't care power, fly mode
#else
    60,60,60, // don't care power, normal mode
#endif
    60,600,300  // power saving
  };

//----- SENSOR CUSTOMIZATION -----
// Sensor README:

#if (APP_ID==(APPTYPE_SYSTEM_BASE+1) || APP_ID==(APPTYPE_PUBLIC_BASE+12))
  #define APP_NAME "PM25" // REPLACE: this is your unique application name 
  
  #ifdef USE_SHT31
    #include <Wire.h>
    #include "sht3x.h"
    SHT3X sht3x;
  #endif
  
  #ifdef USE_DHT22
    #include <DHT_linkit.h>     // Reference: https://github.com/Seeed-Studio/Grove_Starter_Kit_For_LinkIt/tree/master/libraries/Humidity_Temperature_Sensor 
    #define DHTPIN 2
    #define DHTTYPE DHT22   // DHT 22  (AM2302)
    DHT_linkit dht(DHTPIN, DHTTYPE);
  #endif
  #if APP_ID==(APPTYPE_PUBLIC_BASE+12)
    #define APP_NAME "Air-1" // REPLACE: this is your unique application name 
  
    #include <Wire.h>
    #include "MutichannelGasSensor.h"

  #endif
#elif APP_ID==(APPTYPE_PUBLIC_BASE+2)
  #define APP_NAME "EXAMPLE_APP1" // REPLACE: this is your unique application name 
#elif APP_ID==(APPTYPE_PUBLIC_BASE+1)
  #define APP_NAME "Wuulong" // REPLACE: this is your unique application name 
#elif APP_ID==(APPTYPE_PUBLIC_BASE+3)
  #define APP_NAME "MAPS" // REPLACE: this is your unique application name 
  #include <DHT_linkit.h>     // Reference: https://github.com/Seeed-Studio/Grove_Starter_Kit_For_LinkIt/tree/master/libraries/Humidity_Temperature_Sensor
  #include <Digital_Light_TSL2561.h>  // Reference:  http://www.seeedstudio.com/wiki/Grove_-_Digital_Light_Sensor
  #include <HP20x_dev.h>      // Reference: http://www.seeedstudio.com/wiki/Grove_-_Barometer_(High-Accuracy)
  #include <KalmanFilter.h>
  #define DHTPIN 2
  #define DHTTYPE DHT22   // DHT 22  (AM2302)
  DHT_linkit dht(DHTPIN, DHTTYPE);
  unsigned char ret = 0;
  KalmanFilter t_filter;    //temperature filter
  KalmanFilter p_filter;    //pressure filter
  KalmanFilter a_filter;    //altitude filter

#endif



//----- GENERAL -----
#define LED_BLINK_DELAY 200

enum info_type{
    INFO_GPS=1,
    INFO_MQTT=2,
    INFO_LOGFILE=3
};



int current_power_policy=0;

unsigned long currentTime = 0;  // current loop time
unsigned long LastPostTime = 0; // last upload time
unsigned long lastWifiReadyTime = 0; // last wifi ready time | the time be checked




//----- GPS -----
gpsSentenceInfoStruct info;
char buff_tmp[128]; //buffer
char utcstr[32]; //buffer
char datestr[32]; //buffer
double ground_speed;


/// new variables starting from v0.7.0
//char str_GPS_location[60];
char str_GPS_lat[15];
char str_GPS_lon[15];
char str_GPS_quality[5];
char str_GPS_satellite[5];
char str_GPS_altitude[10];


//----- WIFI -----
LWiFiClient wifiClient;
int failedCounter=0;
LWifiStatus wifistatus;

//----- SENSORS -----

char sensorType[SENSOR_CNT][3];
float sensorValue[SENSOR_CNT];
char sensorUploadString[SENSOR_STRING_MAX]; //buffer // Please extend this if you need

// The followings are used for DHT22 sensors
float h,t;
boolean ThreadComplete;

//----- MQTT -----

char mqttTopic[64];
char mqttTopicSelf[64]; // The topic used for central alarm
char mqttTopicPartner[64]; // The topic used for partner alarm
void msgCallback(char* topic, byte* payload, unsigned int len);
PubSubClient mqttClient((char*)MQTT_PROXY_IP, 1883, msgCallback, wifiClient);
char clientID[50]; //buffer
#define MSG_BUFFER_MAX 512
char msg[MSG_BUFFER_MAX]; //buffer



//----- Storage -----
#include <LFlash.h>
#include <LSD.h>
#include <LStorage.h>


#define Drv LFlash          // use Internal 10M Flash
// #define Drv LSD           // use SD card
#define LOG_FILENAME "data.log"
#define SETTING_FILENAME "setting.bin"
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

//----- STATE -----
int wifi_ready=0;
int gps_ready=0; // 1: when satellites number>1, 0: when satellites==0 



int logic_select(int iWhatLogic){
  int ret = 0;
  switch(iWhatLogic){
    case LOGIC_WIFI_NEED_CONNECT:
      // return: (0) no need to connect, (1) need connect
      // policy_power=don' case: when wifi not ready
      // policy_power=save, last send > upload_period, last_wifi alive > wifi_period 
      // policy_online=POLICY_ONLINE_ALWAYS
      if( wifi_ready==0){
        if((POLICY_ONLINE== POLICY_ONLINE_ALWAYS) && (ADJ_MODE==ADJ_MODE_NORMAL)){
          return 1;
        }
        
        //if(current_power_policy==POLICY_POWER_DONTCARE){
        if( LastPostTime ==0 ){ // system init
          return 1;
        }
        if( (currentTime+1000 - lastWifiReadyTime) >(period_target[current_power_policy][PERIOD_WIFICHECK_IDX]*1000)) {
          lastWifiReadyTime = currentTime; // wifi checked, reset wifi ready time to current. 
          if( (currentTime+1000 - LastPostTime) >(period_target[current_power_policy][PERIOD_UPLOAD_IDX]*1000)) {
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
        if( LastPostTime ==0 ){ // system init
          return 1;
        }

        if ((currentTime+1000 - LastPostTime) > (period_target[current_power_policy][PERIOD_UPLOAD_IDX]*1000)) { // all information will show to console, but send out period must > Upload period

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
#if (APP_ID == (APPTYPE_SYSTEM_BASE+1) || APP_ID==(APPTYPE_PUBLIC_BASE+12))
#ifdef USE_DHT22
  dht.begin();  // DHT22
#endif
 
#ifdef USE_SHT31
  sht3x.setAddress(SHT3X::I2C_ADDRESS_44);
  sht3x.setAccuracy(SHT3X::ACCURACY_HIGH);
  Wire.begin(); 
#endif
#if APP_ID==(APPTYPE_PUBLIC_BASE+12)
  setup_mgs();
#endif

#elif APP_ID==(APPTYPE_PUBLIC_BASE+1)
  pinMode(SOUND_SENSOR_PIN, INPUT); 
  pinMode(DUST_SENSOR_PIN, INPUT);

#elif APP_ID == (APPTYPE_PUBLIC_BASE+3)
      // Grove - Temperature and Humidity Sensor Pro
  dht.begin();
  // Grove - Digital Light Sensor
  TSL2561.init();
  // Grove - Barometer (High-Accuracy)
  delay(150);     // Power up,delay 150ms,until voltage is stable
  HP20x.begin();  // Reset HP20x_dev
  delay(100);
  // Determine HP20x_dev is available or not 
  ret = HP20x.isAvailable();
  if(OK_HP20X_DEV == ret){
    Serial.println("HP20x_dev is available.");    
  } else {
    Serial.println("HP20x_dev isn't available.");
  }
#endif
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

long record_id=0;
long debug_wifi=0;
unsigned long duration;
unsigned long dust_starttime=0;
unsigned long dust_sampletime_ms = 30000;//sampe 30s ;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0;

#if APP_ID==(APPTYPE_PUBLIC_BASE+1)
// dust sensor need keep monitor the PWM. This function need dust_sampletime_ms to run, and will block others function
int get_sensor_data_dust(){
  dust_starttime = millis();
  while(1){
    
    duration = pulseInNoWaitStart(DUST_SENSOR_PIN, LOW, dust_sampletime_ms*50);
    //duration = pulseIn(DUST_SENSOR_PIN, LOW);
    lowpulseoccupancy = lowpulseoccupancy+duration;
  
    if ((millis()-dust_starttime) > dust_sampletime_ms)//if the sampel time == 30s
    {
      ratio = lowpulseoccupancy/(dust_sampletime_ms*10.0);  // Integer percentage 0=>100
      concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62; // using spec sheet curve
      Serial.print("\tconcentration = ");
      Serial.print(concentration);
      Serial.println(" pcs/0.01cf");
      Serial.print("\tratio = ");
      Serial.println(ratio);
      Serial.print("\tduration = ");
      Serial.println(duration);

      lowpulseoccupancy = 0;
      dust_starttime = millis();
      break; 
     
    }
   // mqttClient.loop();
    blynk_loop1(); // system hang too long may cause problem for blynk, in quick test, should be less than 5 second.
    delay(50);
  }

}
// get UV sensor data
int get_sensor_data_uv(){
  int value;
  long  sum=0;
  for(int i=0;i<32;i++)
   {  
      value=analogRead(UV_SENSOR_PIN);
      sum=value+sum;
      
      delay(2);
   }   
   sum = sum >> 5;
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

}

// get sound sensor data
int get_sensor_data_sound(){
  long value;
  long max_value=0;
  for(int i=0;i<32;i++) // get some max sound level, because even very loud it can still measure 0 
   {  
      value=analogRead(SOUND_SENSOR_PIN); //use A1 to read the electrical signal
      if(value>max_value){
        max_value = value;
      }
      
      delay(2);
   }  
   return max_value;
   
}
#endif

#if APP_ID == (APPTYPE_PUBLIC_BASE+3)
// get Barometer (HIGH-ACCURACY) sensor data
float get_sensor_data_barometer(){
long temperature, pressure, altitude;
float bt, bp, ba;
int k;
    bp = 0;
    if(OK_HP20X_DEV == ret){ 
      while(1){
        pressure = HP20x.ReadPressure();
        bp = p_filter.Filter(pressure/100.0);
//      altitude = HP20x.ReadAltitude();
//      ba = a_filter.Filter(altitude/100.0);    // the result is not used though.... 
//      temperature = HP20x.ReadTemperature();
//      bt = t_filter.Filter(temperature/100.0);   // the result is not used though.... 
        if (bp>10) break;
        Serial.println("Something wrong with barometer => retry it!");
      }
    } else {
      HP20x.begin();  // Reset HP20x_dev
      delay(100);
      ret = HP20x.isAvailable();
    }
    return bp;
}
#endif

String msg_sensor;
unsigned long timecount;

#ifdef USE_PM25_G3
int pm25sensorG3(){
  unsigned long timeout = millis();
  int count=0;
  byte incomeByte[24];
  boolean startcount=false;
  byte data;
  Serial1.begin(9600);
  while (1){
    if((millis() -timeout) > 3000) {    
      Serial.println("[G3-ERROR-TIMEOUT]");
      //#TODO:make device fail alarm message here
      break;
    }
    if(Serial1.available()){
      data=Serial1.read();
    if(data==0x42 && !startcount){
      startcount = true;
      count++;
      incomeByte[0]=data;
    }else if(startcount){
      count++;
      incomeByte[count-1]=data;
      if(count>=24) {break;}
     }
    }
  }
  Serial1.end();
  Serial1.flush();
  unsigned int calcsum = 0; // BM
  unsigned int exptsum;
  for(int i = 0; i < 22; i++) {
    calcsum += (unsigned int)incomeByte[i];
  }
  
  exptsum = ((unsigned int)incomeByte[22] << 8) + (unsigned int)incomeByte[23];
  if(calcsum == exptsum) {
    count = ((unsigned int)incomeByte[12] << 8) + (unsigned int)incomeByte[13];

    //PM10
    sensorValue[SENSOR_ID_DUST10] = ((unsigned int)incomeByte[14] << 8) + (unsigned int)incomeByte[15];
    
    return count;
  } else {
    Serial.println("#[exception] PM2.5 Sensor CHECKSUM ERROR!");
    return -1;
  }     
}
#endif

#ifdef USE_PM25_A4
int pm25sensorA4(){
  unsigned long timeout = millis();
  int count=0;
  byte incomeByte[32];
  boolean startcount=false;
  byte data;
  Serial1.begin(9600);
  while (1){
    if((millis() -timeout) > 1500) {    
      Serial.println("[A4-ERROR-TIMEOUT]");
      //#TODO:make device fail alarm message here
      break;
    }
    if(Serial1.available()){
      data=Serial1.read();
      if(data==0x32 && !startcount){
        startcount = true;
        count++;
        incomeByte[0]=data;
      }else if(startcount){
        count++;
        incomeByte[count-1]=data;
        if(count>=32) {break;}
      }
    }
  }
  Serial1.end();
  Serial1.flush();
  unsigned int calcsum = 0; // BM
  unsigned int exptsum;
  for(int i = 0; i < 29; i++) {
    calcsum += (unsigned int)incomeByte[i];
  }
  
  exptsum = ((unsigned int)incomeByte[30] << 8) + (unsigned int)incomeByte[31];
  if(calcsum == exptsum) {
    count = ((unsigned int)incomeByte[6] << 8) + (unsigned int)incomeByte[7];

    //PM10
    sensorValue[SENSOR_ID_DUST10] = ((unsigned int)incomeByte[8] << 8) + (unsigned int)incomeByte[9];
    
    return count;
  } else {
    Serial.println("#[exception] PM2.5 Sensor CHECKSUM ERROR!");
    return -1;
  }     
}
#endif

#if APP_ID==(APPTYPE_PUBLIC_BASE+12)

void setup_mgs()
{
    
    mutichannelGasSensor.begin(0x04);//the default I2C address of the slave is 0x04
    //mutichannelGasSensor.changeI2cAddr(0x10);
    mutichannelGasSensor.doCalibrate();
    
    mutichannelGasSensor.powerOn();
    Serial.println("CSV,NH3,CO,NO2,C3H8,C4H10,CH4,H2,C2H5OH");
    
}

float get_sensor_data_mgs(){
    
    sensorValue[SENSOR_ID_NH3] = mutichannelGasSensor.measure_NH3();

    sensorValue[SENSOR_ID_CO] = mutichannelGasSensor.measure_CO();

    sensorValue[SENSOR_ID_NO2] = mutichannelGasSensor.measure_NO2();

    sensorValue[SENSOR_ID_C3H8] = mutichannelGasSensor.measure_C3H8();

    sensorValue[SENSOR_ID_C4H10] = mutichannelGasSensor.measure_C4H10();

    sensorValue[SENSOR_ID_CH4] = mutichannelGasSensor.measure_CH4();

    sensorValue[SENSOR_ID_H2] = mutichannelGasSensor.measure_H2();

    sensorValue[SENSOR_ID_C2H5OH] = mutichannelGasSensor.measure_C2H5OH();


    if(sensorValue[SENSOR_ID_NH3]>=0){
      Serial.print("The concentration of NH3 is ");
      Serial.print(sensorValue[SENSOR_ID_NH3]);
      Serial.println(" ppm");
    }

    if(sensorValue[SENSOR_ID_CO]>=0){
      Serial.print("The concentration of CO is ");
      Serial.print(sensorValue[SENSOR_ID_CO]);
      Serial.println(" ppm");
    }

    if(sensorValue[SENSOR_ID_NO2]>=0){
      Serial.print("The concentration of NO2 is ");
      Serial.print(sensorValue[SENSOR_ID_NO2]);
      Serial.println(" ppm");
    }

    if(sensorValue[SENSOR_ID_C3H8]>=0){
      Serial.print("The concentration of C3H8 is ");
      Serial.print(sensorValue[SENSOR_ID_C3H8]);
      Serial.println(" ppm");
    }

    if(sensorValue[SENSOR_ID_C4H10]>=0){
      Serial.print("The concentration of C4H10 is ");
      Serial.print(sensorValue[SENSOR_ID_C4H10]);
      Serial.println(" ppm");
    }

    if(sensorValue[SENSOR_ID_CH4]>=0){
      Serial.print("The concentration of CH4 is ");
      Serial.print(sensorValue[SENSOR_ID_CH4]);
      Serial.println(" ppm");
    }

    if(sensorValue[SENSOR_ID_H2]>=0){
      Serial.print("The concentration of H2 is ");
      Serial.print(sensorValue[SENSOR_ID_H2]);
      Serial.println(" ppm");
    }

    if(sensorValue[SENSOR_ID_C2H5OH]>=0){
      Serial.print("The concentration of C2H50H is ");
      Serial.print(sensorValue[SENSOR_ID_C2H5OH]);
      Serial.println(" ppm");
    }
    Serial.print("CSV,");
    Serial.print(sensorValue[SENSOR_ID_NH3]);    
    Serial.print(",");
    Serial.print(sensorValue[SENSOR_ID_CO]);
    Serial.print(",");
    Serial.print(sensorValue[SENSOR_ID_NO2]);
    Serial.print(",");
    Serial.print(sensorValue[SENSOR_ID_C3H8]);
    Serial.print(",");
    Serial.print(sensorValue[SENSOR_ID_C4H10]);
    Serial.print(",");
    Serial.print(sensorValue[SENSOR_ID_CH4]);
    Serial.print(",");
    Serial.print(sensorValue[SENSOR_ID_H2]);
    Serial.print(",");
    Serial.println(sensorValue[SENSOR_ID_C2H5OH]);    
}

#endif
// init all sensor data to 0, maybe not necessary
void init_sensor_data(){

  int i;
  for(i=0;i<SENSOR_CNT;i++)
  {
   sensorValue[i]=0;  
   strcpy(sensorType[i], "-");
  }

  strcpy(sensorType[0], "0");
  strcpy(sensorType[1], "1");
  strcpy(sensorType[2], "2");
  strcpy(sensorType[3], "3");
  strcpy(sensorType[4], "4");
#if APP_ID == (APPTYPE_SYSTEM_BASE+1)
  strcpy(sensorType[SENSOR_ID_DUST],"d0");
  strcpy(sensorType[SENSOR_ID_DUST10],"d1");
  strcpy(sensorType[SENSOR_ID_TEMPERATURE], "t0");
  strcpy(sensorType[SENSOR_ID_HUMIDITY],"h0");
#elif APP_ID == (APPTYPE_PUBLIC_BASE+1)
  strcpy(sensorType[SENSOR_ID_DUST],"d0");
  strcpy(sensorType[SENSOR_ID_UV], "u0");
  strcpy(sensorType[SENSOR_ID_SOUND], "s0");
#elif APP_ID == (APPTYPE_PUBLIC_BASE+2)  
  strcpy(sensorType[SENSOR_ID_DUST],"d0");
  strcpy(sensorType[SENSOR_ID_DUST10],"d1");
#elif APP_ID == (APPTYPE_PUBLIC_BASE+3)
  strcpy(sensorType[SENSOR_ID_BAROMETER], "b0");
  strcpy(sensorType[SENSOR_ID_TEMPERATURE], "t0");
  strcpy(sensorType[SENSOR_ID_HUMIDITY],"h0");
  strcpy(sensorType[SENSOR_ID_LIGHT], "l0");
#elif APP_ID == (APPTYPE_PUBLIC_BASE+12)
  strcpy(sensorType[SENSOR_ID_DUST],"d0");
  strcpy(sensorType[SENSOR_ID_DUST10],"d1");
  strcpy(sensorType[SENSOR_ID_TEMPERATURE], "t0");
  strcpy(sensorType[SENSOR_ID_HUMIDITY],"h0");
  strcpy(sensorType[SENSOR_ID_NH3],"g0");
  strcpy(sensorType[SENSOR_ID_CO],"g1");
  strcpy(sensorType[SENSOR_ID_NO2],"g2");
  strcpy(sensorType[SENSOR_ID_C3H8],"g3");
  strcpy(sensorType[SENSOR_ID_C4H10],"g4");
  strcpy(sensorType[SENSOR_ID_CH4],"g5");
  strcpy(sensorType[SENSOR_ID_H2],"g6");
  strcpy(sensorType[SENSOR_ID_C2H5OH],"g7");
#endif
  
}


// please customize the how to get the sensor data and store to sensorValue[]
int get_sensor_data(){
    // sensor 0-9: system sensor
    Serial.print("SensorValue(RecordID):");
    sensorValue[SENSOR_ID_RECORDID]=record_id;
    Serial.println(sensorValue[SENSOR_ID_RECORDID]);     

    getBatteryStatus();
    sensorValue[SENSOR_ID_BATTERYLEVEL] = batteryStatus.batteryLevel;//use A1 to read the electrical signal
    Serial.print("SensorValue(BatteryLevel):");
    Serial.println(sensorValue[SENSOR_ID_BATTERYLEVEL]);
    
    sensorValue[SENSOR_ID_BATTERYCHARGING] = batteryStatus.charging;//use A1 to read the electrical signal
    Serial.print("SensorValue(BatteryCharging):");
    Serial.println(sensorValue[SENSOR_ID_BATTERYCHARGING]);
    
    Serial.print("SensorValue(speed):");
    sensorValue[SENSOR_ID_GROUNDSPEED]=ground_speed;
    Serial.println(sensorValue[SENSOR_ID_GROUNDSPEED]); 

    Serial.print("SensorValue(debugwifi):");
    sensorValue[SENSOR_ID_DEBUGWIFI]=debug_wifi;
    Serial.println(sensorValue[SENSOR_ID_DEBUGWIFI]);     

    //sensor 10-19: user sensor
#if (APP_ID == (APPTYPE_SYSTEM_BASE+1) || APP_ID == (APPTYPE_PUBLIC_BASE+12))
      //Debug Time Count
      Serial.print("[Performence TIME-COUNT]:");
      timecount=millis()-timecount;
      Serial.println(timecount);
      timecount=millis();
      //Debug Time Count
  #ifdef USE_PM25_G3
      sensorValue[SENSOR_ID_DUST] = (float)pm25sensorG3();
      Serial.print("[SENSOR-DUST-PM2.5]:");
      Serial.println(sensorValue[SENSOR_ID_DUST]);
  #endif
  #ifdef USE_PM25_A4
      sensorValue[SENSOR_ID_DUST] = (float)pm25sensorA4();
      Serial.print("[SENSOR-DUST-PM2.5]:");
      Serial.println(sensorValue[SENSOR_ID_DUST]);
  #endif
      //in-code assign value
      Serial.print("[SENSOR-DUST-PM10]:");
      Serial.println(sensorValue[SENSOR_ID_DUST10]);
  #ifdef USE_DHT22  
      ThreadComplete=false;
      LTask.remoteCall(createThread, NULL);
      delay(200);
      while(!ThreadComplete){
        delay(1000);
      };     
  #endif
  #ifdef USE_SHT31
      sht3x.readSample();
      t=sht3x.getTemperature();
      h=sht3x.getHumidity();
  #endif
  
      sensorValue[SENSOR_ID_TEMPERATURE] = t;
      Serial.print("SensorValue(Temperature):");
      Serial.println(sensorValue[SENSOR_ID_TEMPERATURE]);
      sensorValue[SENSOR_ID_HUMIDITY] = h;
      Serial.print("SensorValue(Humidity):");
      Serial.println(sensorValue[SENSOR_ID_HUMIDITY]);
      
#if APP_ID==(APPTYPE_PUBLIC_BASE+12)
  get_sensor_data_mgs();
#endif      
      
#elif APP_ID == (APPTYPE_PUBLIC_BASE+1)  
      Serial.println("Measure dust, take 30 seconds ...");
      get_sensor_data_dust();
      Serial.print("SensorValue(dust sensor):");
      sensorValue[SENSOR_ID_DUST]=concentration;
      Serial.println(sensorValue[SENSOR_ID_DUST]);     
      get_sensor_data_uv();
      sensorValue[SENSOR_ID_UV] = ii;    
      Serial.print("SensorValue(UV sensor):");
      Serial.println(sensorValue[SENSOR_ID_UV]); 
      
      
      sensorValue[SENSOR_ID_SOUND] = get_sensor_data_sound();
      Serial.print("SensorValue(Sound):");
      Serial.println(sensorValue[SENSOR_ID_SOUND]);
#elif APP_ID == (APPTYPE_PUBLIC_BASE+2)
      Serial.print("[Performence TIME-COUNT]:");
      timecount=millis()-timecount;
      Serial.println(timecount);
      timecount=millis();
      sensorValue[SENSOR_ID_DUST] = (float)pm25sensorG3();
      Serial.print("[SENSOR-DUST]:");
      Serial.println(sensorValue[SENSOR_ID_DUST]);
           //in-code assign value
      Serial.print("[SENSOR-DUST-PM10]:");
      Serial.println(sensorValue[SENSOR_ID_DUST10]);
      
#elif APP_ID == (APPTYPE_PUBLIC_BASE+3)
      sensorValue[SENSOR_ID_BAROMETER] = get_sensor_data_barometer();
      Serial.print("SensorValue(Barometer):");
      Serial.println(sensorValue[SENSOR_ID_BAROMETER]);


      ThreadComplete=false;
      LTask.remoteCall(createThread, NULL);
      delay(200);
      while(!ThreadComplete){
        delay(1000);
      };    

 
/*      float h,t;
      dht.readHT(&t, &h);
      while (isnan(t) || isnan(h) || t<0 || t>80 || h<0 || h > 100){
        Serial.println("Something wrong with DHT => retry it!");
        delay(100);
        dht.readHT(&t, &h);    
      }
*/

      sensorValue[SENSOR_ID_TEMPERATURE] = t;
      Serial.print("SensorValue(Temperature):");
      Serial.println(sensorValue[SENSOR_ID_TEMPERATURE]);
      sensorValue[SENSOR_ID_HUMIDITY] = h;
      Serial.print("SensorValue(Humidity):");
      Serial.println(sensorValue[SENSOR_ID_HUMIDITY]);

      sensorValue[SENSOR_ID_LIGHT] = TSL2561.readVisibleLux();
      Serial.print("SensorValue(Light):");
      Serial.println(sensorValue[SENSOR_ID_LIGHT]);
#endif

/*
  msg_sensor = "|values=";
  int i;
  for(i=0;i<SENSOR_CNT;i++)
  {
    if(i>0){
       msg_sensor.concat(",");
    }
    msg_sensor.concat(sensorValue[i]);
  }
*/

  msg_sensor = "";
  int i;
  for(i=0;i<SENSOR_CNT;i++)
  {
    if (sensorType[i][0] != '-'){
      msg_sensor.concat("|s_");
      msg_sensor.concat(sensorType[i]);
      msg_sensor.concat("=");
      msg_sensor.concat(sensorValue[i]);
    }
  }

  if(msg_sensor.length()<SENSOR_STRING_MAX){
    msg_sensor.toCharArray(sensorUploadString, msg_sensor.length()+1);
  }else{
    sensorUploadString[0]=0;
    Serial.println("Sensor string overflow!");
  }
}


#ifdef USE_DHT22
boolean createThread(void* userdata) { 
  // The priority can be 1 - 255 and default priority are 0 
  // the arduino priority are 245 
  vm_thread_create(dht_thread, NULL, 255); 
  return true; 
} 
// This is the thread it self 
VMINT32 dht_thread(VM_THREAD_HANDLE thread_handle, void* user_data) 
{ 
      dht.readHT(&t, &h);
      while (isnan(t) || isnan(h) || t<0 || t>80 || h<0 || h > 100){
        Serial.println("Something wrong with DHT => retry it!");
        delay(100);
        dht.readHT(&t, &h);   
      }
      ThreadComplete=true;
}
#endif

#if BLYNK_ENABLE == 1
// Blynk - Virtual port setup. 
// setup the logic to read your customize sensor data
BLYNK_READ(SENSOR_ID_RECORDID) // sensorValue[0] : Sound
{
  Serial.print("\tBlynk comes to read!");
  Blynk.virtualWrite(SENSOR_ID_RECORDID, sensorValue[SENSOR_ID_RECORDID]);
}

BLYNK_READ(SENSOR_ID_BATTERYLEVEL) // sensorValue[0] : Sound
{
  Blynk.virtualWrite(SENSOR_ID_BATTERYLEVEL, sensorValue[SENSOR_ID_BATTERYLEVEL]);
}

BLYNK_READ(SENSOR_ID_BATTERYCHARGING) 
{
  Blynk.virtualWrite(SENSOR_ID_BATTERYCHARGING, sensorValue[SENSOR_ID_BATTERYCHARGING]);
}

BLYNK_READ(SENSOR_ID_GROUNDSPEED) 
{
  Blynk.virtualWrite(SENSOR_ID_GROUNDSPEED, sensorValue[SENSOR_ID_GROUNDSPEED]);
}

BLYNK_READ(SENSOR_ID_DEBUGWIFI) 
{
  Blynk.virtualWrite(SENSOR_ID_DEBUGWIFI, sensorValue[SENSOR_ID_DEBUGWIFI]);
}


#if APP_ID==(APPTYPE_SYSTEM_BASE+1)
BLYNK_READ(SENSOR_ID_DUST_BLYNK) 
{
  Blynk.virtualWrite(SENSOR_ID_DUST_BLYNK, sensorValue[SENSOR_ID_DUST]);
}
BLYNK_READ(SENSOR_ID_TEMPERATURE_BLYNK) 
{
  Blynk.virtualWrite(SENSOR_ID_TEMPERATURE_BLYNK, sensorValue[SENSOR_ID_TEMPERATURE]);
}
BLYNK_READ(SENSOR_ID_HUMIDITY_BLYNK) 
{
  Blynk.virtualWrite(SENSOR_ID_HUMIDITY_BLYNK, sensorValue[SENSOR_ID_HUMIDITY]);
}
BLYNK_READ(SENSOR_ID_DUST10_BLYNK) 
{
  Blynk.virtualWrite(SENSOR_ID_DUST10_BLYNK, sensorValue[SENSOR_ID_DUST10]);
}

#elif APP_ID==(APPTYPE_PUBLIC_BASE+1)
BLYNK_READ(SENSOR_ID_DUST_BLYNK) 
{
  Blynk.virtualWrite(SENSOR_ID_DUST_BLYNK, sensorValue[SENSOR_ID_DUST]);
}
BLYNK_READ(SENSOR_ID_UV_BLYNK) 
{
  Blynk.virtualWrite(SENSOR_ID_UV_BLYNK, sensorValue[SENSOR_ID_UV]);
}
BLYNK_READ(SENSOR_ID_SOUND_BLYNK) 
{
  Blynk.virtualWrite(SENSOR_ID_SOUND_BLYNK, sensorValue[SENSOR_ID_SOUND]);
}
#elif APP_ID==(APPTYPE_PUBLIC_BASE+2)
BLYNK_READ(SENSOR_ID_DUST_BLYNK) 
{
  Blynk.virtualWrite(SENSOR_ID_DUST_BLYNK, sensorValue[SENSOR_ID_DUST]);
}

BLYNK_READ(SENSOR_ID_DUST10_BLYNK) 
{
  Blynk.virtualWrite(SENSOR_ID_DUST10_BLYNK, sensorValue[SENSOR_ID_DUST10]);
}

#elif APP_ID==(APPTYPE_PUBLIC_BASE+3)
BLYNK_READ(SENSOR_ID_BAROMETER_BLYNK) 
{
  Blynk.virtualWrite(SENSOR_ID_BAROMETER_BLYNK, sensorValue[SENSOR_ID_BAROMETER]);
}
BLYNK_READ(SENSOR_ID_TEMPERATURE_BLYNK) 
{
  Blynk.virtualWrite(SENSOR_ID_TEMPERATURE_BLYNK, sensorValue[SENSOR_ID_TEMPERATURE]);
}
BLYNK_READ(SENSOR_ID_HUMIDITY_BLYNK) 
{
  Blynk.virtualWrite(SENSOR_ID_HUMIDITY_BLYNK, sensorValue[SENSOR_ID_HUMIDITY]);
}
BLYNK_READ(SENSOR_ID_LIGHT_BLYNK) 
{
  Blynk.virtualWrite(SENSOR_ID_LIGHT_BLYNK, sensorValue[SENSOR_ID_LIGHT]);
}
#endif

#endif

#if ALARM_ENABLE == 1
//----- ALARM CUSTOMIZATION
void alarm_setup(){
  analogWrite(BUZZER_ALARM_PIN, 0);
  pinMode(BUZZER_ALARM_PIN, OUTPUT);
}

// alarm decided by local sensor
void alarm_self_handler(){
  //check the sensor data if needed, make a alarm
}

// alarm: (0) off, (1) on
void alarm_buzzer_set(int alarm){
  int outputValue = 0;
  // map it to the range of the analog out:
  //outputValue = map(sensorValue, 0, 1023, 0, 255);  
  if( alarm ){
    outputValue=128;
  }else{
    outputValue=0;
  }
  // change the analog out value:
//  analogWrite(BUZZER_ALARM_PIN, outputValue);           


}

void alarmHandlerCentral(byte* payload,unsigned int len){

  // early stage alarm handler
  if(payload[0]=='|' && payload[1]=='c' && payload[2]=='m' && payload[3]=='d' && payload[4]=='='){
    //valid cmd
    if(payload[5]=='o'){ //alarm on
      alarm_buzzer_set(1);
      Serial.println("cmd: alarm set");

    }
    if(payload[5]=='f'){ //alarm off
      alarm_buzzer_set(0);
      Serial.println("cmd: alarm off");
    }
  }
}

void alarmHandlerPartner(byte* payload,unsigned int len){
  // The message here should be the same with normal msg we publish
  //|ver_format=1|app=EXAMPLE_APP|ver_app=0.6|device_id=LASS-Example|tick=117122|date=6/1/80|time=0:1:36|device=LinkItONE|values=18.00,100.00,1.00,872.12,0.62,0.00|gps=$GPGGA,000136.000,2448.0018,N,12059.6686,E,0,0,,135.0,M,15.0,M,,*46\r
  char *pstr = strstr((char*)payload,"values=");
  if(pstr==NULL){
    return;
  }
  // find sensor value
  char *pstr_end = strstr(pstr,"|");
  if(pstr_end==NULL){
    //not handle values at string end yet
    return;
  }
  
  int partnerSensorId=1;
  int tmp = getComma(partnerSensorId,pstr);
  double sensor_value = getDoubleNumber(pstr+tmp);
  // decision logic
  // alarm logic
  Serial.print("Partner's sensors value=");
  Serial.println(sensor_value);
}

#else
void alarm_setup(){}
void alarm_self_handler(){}

#endif
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

String msg_tmp; 
// pack different type's information, print it for debug
void packInfo(int infoType){
  

  switch(infoType){
    case INFO_GPS:
      if(FAKE_GPS==0){ // there is a GPS
        Serial.print("GPS raw data(GPRMC):");
        Serial.print((char*)info.GPRMC);
        Serial.print("GPS raw data(GPGGA):");
        Serial.print((char*)info.GPGGA);
        Serial.print("GPS raw data(GPVTG):");
        Serial.print((char*)info.GPVTG);
        parseGPGGA((const char*)info.GPGGA);
        parseGPRMC((const char*)info.GPRMC);    
        parseGPVTG((const char*)info.GPVTG);
      }else{          // there are no GPS
        //parseGPGGA((const char*)GPS_FIX_INFOR);    //Dont Use AnyMore
        gps_ready=1;
        strcpy(str_GPS_lat,gps_lat);
        strcpy(str_GPS_lon,gps_lon);
        strcpy(str_GPS_altitude,gps_alt);
        strcpy(str_GPS_quality,"1");
        strcpy(str_GPS_satellite,"9");
        
        unsigned long epoch = epochSystem + millis() / 1000;
        int year, month, day, hour, minute, second;
        getCurrentTime(epoch, &year, &month, &day, &hour, &minute, &second);
        sprintf(datestr,"%02d-%02d-%02d",year,month,day);  // use the UTC format for datestr
        sprintf(utcstr,"%02d:%02d:%02d",hour,minute,second); // force each number to have two digits
      }
      // parseGPGGA((const char*)info.GPGGA);
      break;
  
    case INFO_MQTT:
      // mqtt library limit the packet size = 200
      msg_tmp = "|ver_format=";
      msg_tmp.concat(VER_FORMAT);
      msg_tmp.concat("|FAKE_GPS=");
      msg_tmp.concat(FAKE_GPS);
      msg_tmp.concat("|app=");
      msg_tmp.concat(APP_NAME);
      msg_tmp.concat("|ver_app=");
      msg_tmp.concat(VER_APP);
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
      // v0.7.0, added for future integration with backend DB       
      msg_tmp.concat("|gps_lat=");
      msg_tmp.concat(str_GPS_lat);
      msg_tmp.concat("|gps_lon=");
      msg_tmp.concat(str_GPS_lon);
      msg_tmp.concat("|gps_fix=");
      msg_tmp.concat(str_GPS_quality);
      msg_tmp.concat("|gps_num=");
      msg_tmp.concat(str_GPS_satellite);
      msg_tmp.concat("|gps_alt=");
      msg_tmp.concat(str_GPS_altitude);
      msg_tmp.concat(" ");
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
      //Serial.print("Msg length=");
      //Serial.println(msg_tmp.length());
      
      
      if(msg_tmp.length() < MSG_BUFFER_MAX-1){
          msg_tmp.toCharArray(msg, msg_tmp.length()); // the last char will be NULL, design to replace \n
      }else{
        msg[0]=0;
        Serial.println("MSG buffer overflow! Length = " + msg_tmp.length());
      }
    
        break; 
     case INFO_LOGFILE:
      // DATA format: @msg
      // Currently, only support one topic in the log
      dataString="";
      dataString.concat("@");
      dataString.concat(msg);
      // print to the serial port too:
      //Serial.println("Pack Offline log:");
      //Serial.println(dataString);
     
       break;
  }
}

String Record="";

// if data logged, send it out and delete it.
void logSend(){
  int dotCnt=0;
  boolean bConnected;
  int upload_fail=0;

  //Drv.remove((char*)LOG_FILENAME); // for debug
  // upload log only when wifi ready
    if(Drv.exists((char*)LOG_FILENAME)){
      Serial.println("Log exist! Send logging sensor records!");   
      if (!mqttClient.connected()) {
        Serial.println("Reconnecting to MQTT Proxy");
        bConnected = mqttClient.connect(clientID);
        if(bConnected==false){
          Serial.println("Reconnecting to MQTT Proxy: Fail!");
        }
        mqttSubscribeRoutine();
      } 
      
      // if log exist, send
      
      // re-open the file for reading:
      LFile myFile = Drv.open(LOG_FILENAME);
      if (myFile) {
          Serial.println(LOG_FILENAME);
          myFile.seek(0);
          Record="@";
          // read from the file until there's nothing else in it:
          while (myFile.available()) {
              char c= myFile.read();
              if(c=='@'){
                if(Record!="@"){
                  Record.toCharArray(msg, Record.length()+1);
                  Serial.println(msg);
                  if(checkWifiConnected()==1){
                    mqttPublishRoutine(0);
                    
                    LastPostTime = currentTime;
  
                    //mqttClient.publish(mqttTopic, msg);
                    //sent the same msg to partner which may monitor this topic, current work around
                    //these delay msg may cause problem to partner, not send it now.
                    //mqttClient.publish(mqttTopicSelf, msg);
                    Serial.print(".");
                    dotCnt++;
                    if((dotCnt % 1) ==0){
                      Serial.println(".");
                    }
                  }else{
                    upload_fail=1; 
                    break; 
                  }
                  
                }
                Record="";
              }
              else Record.concat(c);            
              //Serial.write(c);
              //Serial.write("!");
          }
          //mqttClient.disconnect();
          // close the file:
          myFile.close();
          if(upload_fail==0){
            Drv.remove((char*)LOG_FILENAME);
            Serial.println("\nUpload complete, log file removed");
          }else{
            Serial.println("\nUpload fail, log file not removed");

          }
          logRecordCnt=0;
      } // if (myFile) 
    } // if(Drv.exists((char*)LOG_FILENAME))

}

// ----flash----
// init setting from code, buffer be clear in the beginning
void setting_init(){
  //clear buffer
  memset((void*)&setting,0,sizeof(SETTING));
  
  //init
  setting.set_version=SETTING_VERSION;
  
  strcpy(setting.wifi_ssid, WIFI_SSID);
  strcpy(setting.wifi_pass, WIFI_PASS);
  setting.wifi_auth = WIFI_AUTH;
	
  strcpy(setting.device_type, DEVICE_TYPE);
  strcpy(setting.device_id, DEVICE_ID);
  strcpy(setting.mqtt_topic_prefix, MQTT_TOPIC_PREFIX);
  strcpy(setting.partner_id,MQTT_TOPIC_PREFIX );
	
  //strcpy(setting.gps_fix_infor, GPS_FIX_INFOR); //not modified...
  strcpy(setting.blynk_auth, blynk_auth);
}
// save setting to flash, will delete old one first.
int setting_save(){
  char* ptr;
  uint8_t byte1;

  //always have new save
  Drv.remove((char*)SETTING_FILENAME);

  // if the file is available, write to it:
  LFile settingFile = Drv.open(SETTING_FILENAME, FILE_WRITE);
  
  if (settingFile) {
    //Serial.println("Save-Opened!");
    settingFile.seek(0);
    //Serial.println(dataString);
    ptr = (char*)&(setting.set_version);
    for(int i=0;i<sizeof(SETTING);i++){
      byte1 = *ptr++;
      settingFile.write(byte1);
      //Serial.println(byte1);
    }
    //settingFile.print(dataString); // record not include \n
    settingFile.close();
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening setting file");
  }

}

// setting load from flash, buffer be clear first to make sure setting is from flash.
int setting_load(){
  uint8_t byte1;
  char* ptr;
  memset((void*)&setting,0,sizeof(SETTING));
  LFile settingFile = Drv.open(SETTING_FILENAME);
  if (settingFile) {
    Serial.println("Load-Opened");
    //Serial.println(SETTING_FILENAME);
    settingFile.seek(0);
    ptr = (char*)&(setting.set_version);
    while (settingFile.available()) {            
      byte1 = settingFile.read();
      //Serial.println(byte1);
      memset(ptr++,byte1,1);
    }
    // close the file:
    settingFile.close();
    return 1;
  } else {
    return 0;
  }
}

// show current setting from setting memory
void setting_show(){
  
  Serial.println("-------------------- User Setting Show --------------------");
  Serial.print("SET_VERSION=");
  Serial.println(setting.set_version);

  Serial.print("SSID=");
  Serial.print(setting.wifi_ssid);
  Serial.print(",WIFI_PASS=");
  Serial.print(setting.wifi_pass);
  Serial.print(",WIFI_AUTH=");
  Serial.println(setting.wifi_auth);

  Serial.print("DeviceType=");
  Serial.print(setting.device_type);
  Serial.print(", DeviceID=");
  Serial.print(setting.device_id);
  Serial.print(", MqttTopicPrefix=");
  Serial.println(setting.mqtt_topic_prefix);
  /*
  Serial.print("GPS_FIX_INFOR=");
  Serial.print(setting.gps_fix_info_lat);
  Serial.print(",");
  Serial.print(setting.gps_fix_info_lot);
  Serial.print(",");
  Serial.println(setting.gps_fix_info_alt);
  */
  Serial.print("BLYNK_AUTH=");
  Serial.println(setting.blynk_auth);
  

}
// simple verification function for setting load/save/print

int setting_verify(){
  setting_init();
  Serial.println("User setting after init!");
  setting_show();
  setting_save();
  Serial.println("User setting after save!");
  setting_show();
  setting_load();
  Serial.println("User setting after load!");
  setting_show();
}
//
int checkWifiConnected(){
    wifistatus = LWiFi.status();
    if (wifistatus==LWIFI_STATUS_CONNECTED){
        return 1;
    }else{
      return 0;
    }
}
// handler after wifi connected, should be called that wifi may just connected
// ex: send log data
void wifiConnected(){ 
    if(wifi_ready) {
      lastWifiReadyTime = currentTime;
      // Log send
      if(logic_select(LOGIC_LOG_NEED_SEND)){
        logSend();
      }
      blynk_setup();  
    }
}

// connecting wifi
void wifiConnecting(){
    // Wifi
    Serial.print("Connecting to Wifi, SSID: ");
    Serial.println(setting.wifi_ssid);
    // Connect to WiFi (max retry 1 times )

    wifi_ready = Mtk_Wifi_Setup_TryCnt(1);
    if(wifi_ready) {
      //printWifiStatus(); // This may crash system, don't know why
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

// light blink to indicate system boot ready and start user code
void lightBlink(){
   digitalWrite(ARDUINO_LED_PIN,LOW);
   delay(LED_BLINK_DELAY);
   digitalWrite(ARDUINO_LED_PIN,HIGH);
   delay(LED_BLINK_DELAY);
   digitalWrite(ARDUINO_LED_PIN,LOW);
}
//-----NTP-----

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
  Serial.println(F("sending NTP packet..."));
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}


#define LEAP_YEAR(Y)     ( ((1970+Y)>0) && !((1970+Y)%4) && ( ((1970+Y)%100) || !((1970+Y)%400) ) )
static const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31}; // API starts months from 1, this array starts from 0

void getCurrentTime(unsigned long epoch, int *year, int *month, int *day, int *hour, int *minute, int *second) {
  int tempDay = 0;

  *hour = (epoch  % 86400L) / 3600;
  *minute = (epoch  % 3600) / 60;
  *second = epoch % 60;

  *year = 1970;
  *month = 0;
  *day = epoch / 86400;

  for (*year = 1970; ; (*year)++) {
    if (tempDay + (LEAP_YEAR(*year) ? 366 : 365) > *day) {
      break;
    } else {
      tempDay += (LEAP_YEAR(*year) ? 366 : 365);
    }
  }
  tempDay = *day - tempDay; // the days left in a year
  for ((*month) = 0; (*month) < 12; (*month)++) {
    if ((*month) == 1) {
      if (LEAP_YEAR(*year)) {
        if (tempDay - 29 < 0) {
          break;
        } else {
          tempDay -= 29;
        }
      } else {
        if (tempDay - 28 < 0) {
          break;
        } else {
          tempDay -= 28;
        }
      }
    } else {
      if (tempDay - monthDays[(*month)] < 0) {
        break;
      } else {
        tempDay -= monthDays[(*month)];
      }
    }
  }
  if(tempDay+2 >monthDays[(*month)]){
    *day = (tempDay+2) -monthDays[(*month)];
    (*month)+=2;
    if((*month)>12){ 
      *year+=1;
      (*month)=(*month)-12;
    }
  } else {
   (*month)++;
   *day = tempDay+2; // one for base 1, one for current day
  }
}

void  retrieveNtpTime(){
  int cb=0;
  while(!cb){
    Serial.println(F("Request NTP..."));
    sendNTPpacket(timeServerIP);
    delay(2000);
    cb = Udp.parsePacket();
  }
  Serial.print(F("packet received, length="));
  Serial.println(cb);
  // We've received a packet, read the data from it
  Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

  //the timestamp starts at byte 40 of the received packet and is four bytes,
  // or two words, long. First, esxtract the two words:

  unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
  unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
  // combine the four bytes (two words) into a long integer
  // this is NTP time (seconds since Jan 1 1900):
  unsigned long secsSince1900 = highWord << 16 | lowWord;
  Serial.print(F("Seconds since Jan 1 1900 = "));
  Serial.println(secsSince1900);

  // now convert NTP time into everyday time:
  Serial.print(F("Unix time = "));
  // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
  const unsigned long seventyYears = 2208988800UL;
  // subtract seventy years:
  epochSystem = secsSince1900 - seventyYears;
  // print Unix time:
  Serial.println(epochSystem);
  
}


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
    
    sprintf(buff_tmp, "\tUTC timer %2d-%2d-%2d", hour, minute, second);
    // sprintf(utcstr,"%d:%d:%d",hour,minute,second);
    sprintf(utcstr,"%02d:%02d:%02d",hour,minute,second); // force each number to have two digits
    Serial.println(buff_tmp);
    
    tmp = getComma(2, GPGGAstr);
    latitude = getDoubleNumber(&GPGGAstr[tmp]);
    tmp = getComma(4, GPGGAstr);
    longitude = getDoubleNumber(&GPGGAstr[tmp]);
    sprintf(buff_tmp, "\tlatitude = %10.4f, longitude = %10.4f", latitude, longitude);
    Serial.println(buff_tmp); 
    sprintf(str_GPS_lat,"%5.6f", latitude/100);
    sprintf(str_GPS_lon,"%5.6f", longitude/100);

    tmp = getComma(6, GPGGAstr);
    num = getIntNumber(&GPGGAstr[tmp]);    
    sprintf(buff_tmp, "\tGPS fix quality = %d", num);
    sprintf(str_GPS_quality,"%d",num);
    
    tmp = getComma(7, GPGGAstr);
    num = getIntNumber(&GPGGAstr[tmp]);    
    sprintf(buff_tmp, "\tsatellites number = %d", num);
    sprintf(str_GPS_satellite,"%d",num);

    tmp = getComma(9, GPGGAstr);
    num = getIntNumber(&GPGGAstr[tmp]);    
    sprintf(buff_tmp, "\taltitude above mean sea level = %d meters", num);
    sprintf(str_GPS_altitude,"%d",num);

    if(num>0 || GPS_SIGNAL_NOCHECK ) {
      gps_ready=1;
    }else{
      gps_ready=0;
    }
    Serial.println(buff_tmp); 
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
    Serial.print("GPS RMC detail result:");
    
    tmp = getComma(9, GPRMCstr);
    day     = (GPRMCstr[tmp + 0] - '0') * 10 + (GPRMCstr[tmp + 1] - '0');
    month   = (GPRMCstr[tmp + 2] - '0') * 10 + (GPRMCstr[tmp + 3] - '0');
    year    = (GPRMCstr[tmp + 4] - '0') * 10 + (GPRMCstr[tmp + 5] - '0');
    
    sprintf(buff_tmp, "\tDate(DD/MM/YY):%d/%d/%d", day, month, year);
    // sprintf(datestr,"%d/%d/%d",day,month,year);
    sprintf(datestr,"20%02d-%02d-%02d",year,month,day);  // use the UTC format for datestr
    Serial.println(buff_tmp);
  }
  else
  {
    Serial.println("Not get data"); 
  }
}

// currently, this function get data's date information.
void parseGPVTG(const char* GPstr)
{
  /* Refer to http://www.gpsinformation.org/dale/nmea.htm#VTG
VTG - Velocity made good. The gps receiver may use the LC prefix instead of GP if it is emulating Loran output.

  $GPVTG,054.7,T,034.4,M,005.5,N,010.2,K*48

where:
        VTG          Track made good and ground speed
        054.7,T      True track made good (degrees)
        034.4,M      Magnetic track made good
        005.5,N      Ground speed, knots
        010.2,K      Ground speed, Kilometers per hour
        *48          Checksum  */
  int tmp;
  
  if(GPstr[0] == '$')
  {
    Serial.print("GPS VTG detail result:");

    tmp = getComma(7, GPstr);
    ground_speed = getDoubleNumber(&GPstr[tmp]);
    
    sprintf(buff_tmp, "\tGround Speed:%f", ground_speed);

    Serial.println(buff_tmp);
  }
  else
  {
    Serial.println("Not get data"); 
  }

}
// return - 0: retry and timeout, 1: success

/* Un-used force reboot
#ifdef use_forcereboot
boolean everlink=false;
#include "vmpwr.h"
boolean reboot(void* userdata)
{
  vm_reboot_normal_start();//vm fuction to reboot
  return true;
}
#endif
*/

int Mtk_Wifi_Setup_TryCnt(int tryCnt) {

    debug_wifi++;
    // -- v0.7.10: wifi exception handling
    /*
    if (mqttClient.connected()){
      mqttClient.disconnect();
    }*/
    wifistatus = LWiFi.status();
    
    if (wifistatus!=LWIFI_STATUS_DISABLED && failedCounter>3){
        Serial.println("before LWiFi.end()...");
        LWiFi.end();
        LWiFi.begin();
        failedCounter = 0;
        Serial.println("after LWiFi.begin() ...("+String(failedCounter, DEC)+")"); 
    } else if (wifistatus==LWIFI_STATUS_DISABLED){
        LWiFi.begin();
        Serial.println("after LWiFi.begin()...("+String(failedCounter, DEC)+")"); 
    } else if (wifistatus==LWIFI_STATUS_CONNECTED){
      Serial.println("---> WiFi status: LWIFI_STATUS_CONNECTED, No need to reconnect!!");  
      return 1;
    }
    
    // attempt to connect to Wifi network:
    //LWiFi.begin();
    // -- end of v0.7.10 changes
    
    int i=0;
    LWiFiEncryption wifi_auth;
    switch(setting.wifi_auth){
      case 0:
        wifi_auth=LWIFI_OPEN;
        memset(setting.wifi_pass, 0, sizeof(setting.wifi_pass));
      break;
      case 1:
        wifi_auth=LWIFI_WEP;
      break;
      case 2:
        wifi_auth=LWIFI_WPA;
      break;
    }

    String wifi_pass=setting.wifi_pass;
    wifi_pass.trim();
    //while (!LWiFi.connectWPA(pSSID, pPassword)) {
    while(LWiFi.connect(setting.wifi_ssid, LWiFiLoginInfo(wifi_auth, wifi_pass))<=0){
        delay(1000);
        Serial.println("retry WiFi AP");
        i++;

        /*
        // -- v0.7.10: wifi exception handling
        wifistatus = LWiFi.status();
        if (wifistatus == LWIFI_STATUS_DISABLED){
          failedCounter++;
          Serial.println("WiFi status: LWIFI_STATUS_DISABLED ("+String(failedCounter, DEC)+")");
        } else if (wifistatus == LWIFI_STATUS_DISCONNECTED) {
          failedCounter++;
          Serial.println("WiFi status: LWIFI_STATUS_DISCONNECTED ("+String(failedCounter, DEC)+")");
        } else {
          Serial.println("WiFi status: LWIFI_STATUS_CONNECTED");
          Serial.print("My IP is: ");
          Serial.println(LWiFi.localIP());
          Serial.print("RSSI is: ");
          Serial.println(LWiFi.RSSI());
          //failedCounter = 0;
        }
        // -- end of v0.7.10 changes
        */
        if(i>=tryCnt){
          //LWiFi.end();
          return 0;
        }
    }
    
    return 1;
}

//----- Battery -----
void getBatteryStatus(){
    batteryStatus.batteryLevel = LBattery.level();
    //sprintf(buff_tmp,"battery level = %d", batteryStatus.batteryLevel );
    //Serial.println(buff_tmp);

    batteryStatus.charging = LBattery.isCharging();
    //sprintf(buff_tmp,"is charging = %d", batteryStatus.charging);
    //Serial.println(buff_tmp);

}

//----- MQTT -----
#if ALARM_ENABLE == 1
void msgDisplay(char* topic, byte* payload, unsigned int len){
  int i;
  char c;
  String str_payload;
  Serial.print("MQTT callback: Topic=");
  Serial.println(topic);
  //Serial.print("callback len:");
  //Serial.println(len);
  //Serial.print(",payload=");
  //for(i=0;i<len;i++){
  //  c = payload[i];
  //  str_payload.concat(c);
  //}
  //should not use the same buffer.
  //str_payload.toCharArray(msg, str_payload.length()+1);
  //Serial.println(msg);

}

void mqttPrintCurrentMsg(){
      Serial.print("Pack MQTT Topic:");
      Serial.println(mqttTopic);
      Serial.println(msg);
}

void mqttSubscribeRoutine(){
/*
  if (mqttClient.connected()){
    mqttClient.subscribe((char*)mqttTopicSelf);
    mqttClient.subscribe((char*)mqttTopicPartner);
  
    Serial.print("Subscribing ");
    Serial.println(mqttTopicSelf);
    Serial.print("Subscribing ");
    Serial.println(mqttTopicPartner);
  }
*/
}
#else
void mqttSubscribeRoutine(){}
#endif

void mqttPublishRoutine(int bPartner){
      // debug the payload limit code
      /*
      uint16_t i,j;
      for(j=0;j<25;j++){
        for(i=0;i<10;i++){
          msg[j*10+i]=48+i;
        }
      }
      msg[j*10]=0;
      */
      //-- v0.7.10
      wifistatus = LWiFi.status();
      if (wifistatus != LWIFI_STATUS_CONNECTED){
        wifi_ready = 0;
        failedCounter++;
        Serial.println("Did not try MQTT PUBLISH because it's not connected....");
        Serial.println();
        return;
      }

      if (mqttClient.connected()){
        mqttClient.publish((char*)mqttTopic, msg);
        if(bPartner){
          mqttClient.publish((char*)mqttTopicSelf, msg);
          Serial.println("MQTT Companion channel published...");
        }
        delay(100);
        if (mqttClient.connected()){
          failedCounter = 0;
        } else {
          failedCounter++;
          Serial.println("Connection to MQTT server failed ("+String(failedCounter, DEC)+")");   
        }
      } else {
        Serial.println("Connection to MQTT server failed ("+String(failedCounter, DEC)+")");   
        failedCounter++;
      }
      
#if ALARM_ENABLE == 1

#endif
      mqttClient.loop();
      Serial.println("MQTT sending");
  
}

String stringTopicCmp = "";

// callback to handle incomming MQTT messages
void msgCallback(char* topic, byte* payload, unsigned int len) { 
#if ALARM_ENABLE == 1
  msgDisplay(topic, payload,len);
  
  // Central alarm
  stringTopicCmp = (char*) mqttTopicSelf;
  if (stringTopicCmp.compareTo(topic) == 0 ) {
    alarmHandlerCentral(payload,len); 
  }
  
  // Partner alarm
  stringTopicCmp = (char*) mqttTopicPartner;
  if (stringTopicCmp.compareTo(topic) == 0 ) {
    alarmHandlerPartner(payload,len); 
  }
#endif  
  
}


#if BLYNK_ENABLE == 1
//----- Blynk -----
bool blynk_connected = false;
/*
// This function is used by Blynk to receive data
size_t BlynkStreamRead(void* buf, size_t len)
{
  return Serial.readBytes((byte*)buf, len);
}

// This function is used by Blynk to send data
size_t BlynkStreamWrite(const void* buf, size_t len)
{
  return Serial.write((byte*)buf, len);
}
*/
// this should be run after wifi connected
void blynk_setup(){
  if(wifi_ready){
    //Blynk.begin(blynk_auth, WIFI_SSID, WIFI_PASS, WIFI_AUTH);
    Blynk.config(blynk_auth);
    blynk_connected=true;
  }
}

// this need to called routinely.
void blynk_loop1(){
  if(wifi_ready){
    if (blynk_connected) {
     Blynk.run();
     Serial.print("B");
    }
  }
  
}
#else
void blynk_setup(){}
void blynk_loop1(){}
#endif

String topicTmp="";
// display current setting information to the console
void display_current_setting(){
  
  topicTmp="";
  topicTmp.concat(MQTT_TOPIC_PREFIX);
  topicTmp.concat("/");
  topicTmp.concat(APP_NAME);
  topicTmp.toCharArray(mqttTopic, topicTmp.length()+1);
  
  topicTmp="";
  topicTmp.concat(mqttTopic);
  topicTmp.concat("/");
  topicTmp.concat(setting.device_id);
  topicTmp.toCharArray(mqttTopicSelf, topicTmp.length()+1);

  topicTmp="";
  topicTmp.concat(mqttTopic);
  topicTmp.concat("/");
  topicTmp.concat(PARTNER_ID);
  topicTmp.toCharArray(mqttTopicPartner, topicTmp.length()+1);
  
  // General
  
  Serial.print("-------------------- LASS V");
  Serial.print(VER_APP);
  Serial.println(" -------------------------");
  Serial.println("User configuration");
  Serial.print("SSID=");
  Serial.print(WIFI_SSID);
  Serial.print(", MQTT_IP=");
  Serial.print(MQTT_PROXY_IP);
  Serial.print(", DeviceID=");
  Serial.print(setting.device_id);
  Serial.print(", PartnerID=");
  Serial.print(PARTNER_ID);
  Serial.print(", TOPIC=");
  Serial.print(mqttTopic);
  Serial.print(", TOPIC_SELF=");
  Serial.print(mqttTopicSelf);
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

String clientIDStr;

//-----Web Setup Page Server
String readString = String(100); //string for fetching data from address
boolean settings_modified=false;

  // send web page
void SendWebPage( LWiFiClient client)
{ 
  String connstr;
  connstr+="HTTP/1.1 200 OK\n"; 
  connstr+="Content-Type: text/html\n"; 
  connstr+="Cache-Control: no-cache, no-store, must-revalidate\n";
  connstr+="Pragma: no-cache\n";
  connstr+="Expires: 0\n";
   // to specify the length, wooul have to construct the entire string and then get its length
   //client.println("Content-Length: 1234"); 
  connstr+="Connnection: close\n\n";
  
  //Start of webpage  connstr+="";
  connstr+="<!DOCTYPE html>\n";
  connstr+="<html><head><title>LASS Setup Page</title></head>\n";
  connstr+="<body marginwidth=\"0\" marginheight=\"0\" \"leftmargin=\"0\" style=\"margin: 0; padding: 0;\">\n";
  connstr+="<table bgcolor=\"#999999\" border\"=\"0\" width=\"100%\" cellpadding=\"1\">\n";
  connstr+="<tr><td>LASS Setup Page</td></tr>\n";
  connstr+="</table>\n";
  connstr+="<p>\n";
  connstr+="<table><form><input type=\"hidden\" name=\"SBM\" value=\"1\">\n";
  connstr+="<tr><td>\n";
  connstr+="DEVICE_ID: \n";
  connstr+="<input type=\"text\" size=\"7\" maxlength=\"7\" name=\"device_id\" value=\"";
  connstr+=setting.device_id;
  connstr+="\"></td></tr>\n";
  connstr+="<tr><td>WiFi SSID: <input type=\"text\" name=\"ssid\" value=\"";
  connstr+=setting.wifi_ssid;
  connstr+="\">\n";
  connstr+="</td></tr>\n";
  connstr+="<tr><td>\n";
  connstr+="WiFi Security:\n";
  connstr+="<select required id=\"security\" name=\"security\">\n";  
    //OPEN 0 WEP 1 WPA 2

  connstr+="<option value=\"0\" ";
  if(setting.wifi_auth==0){
    connstr+="selected";  
  }
  connstr+=">None(Open)</option>\n";
  
  connstr+="<option value=\"1\" ";
  if(setting.wifi_auth==1){
    connstr+="selected";
  }
  connstr+=">WEP</option>\n";  
  
  connstr+="<option value=\"2\" ";  
  if(setting.wifi_auth==2){
    connstr+="selected";
  }
  connstr+=">WPA/WPA2</option></select>\n";
  connstr+="</td></tr>\n";
  connstr+="<tr><td>\n";
  connstr+="WiFi Password:\n";
  connstr+="<input id=\"password\" type=\"text\" name=\"password\" value=\"";
  connstr+=setting.wifi_pass;
  if(setting.wifi_auth==0){
    connstr+="\" disabled=\"true\">\n";
  } else {
    connstr+="\">\n";
  }
  connstr+="</td></tr>\n";
  connstr+="<tr><td>\n";
  connstr+="<input type=\"checkbox\" checked>MTK shouldn't specify NCU as \"OTHER SCHOOLS\" in their recruiting system <br>";
  connstr+="&nbsp;\n";
  connstr+="</td></tr>\n";
  connstr+="<tr><td>\n";
  connstr+="<input id=\"button1\" type=\"submit\" value=\"SUBMIT\">\n";
  if(settings_modified){
   connstr+="-------------Saved to flash!\n";
  }
  connstr+="</td></tr>\n";
  connstr+="</form>\n";
  
  connstr+="<script language=\"javascript\">\n";
  connstr+="document.getElementById('security').addEventListener('change', function() {\n";
  connstr+="if (this.selectedIndex == 0) {\n";
  connstr+="document.getElementById('password').disabled = true;\n";
//  connstr+="document.getElementById('password').value = \"\"\n";
  connstr+="} else {\n";
  connstr+="document.getElementById('password').disabled = false;\n";
  connstr+="}});\n";
  connstr+="</script>\n";
  
  connstr+="</table></body></html>\n";
  //End of webpage
  client.print(connstr);
}

void printwebWifiStatus()
{
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(LWiFi.SSID());
  
  // print your WiFi shield's IP address:
  IPAddress ip = LWiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  Serial.print("subnet mask: ");
  Serial.println(LWiFi.subnetMask());

  Serial.print("gateway IP: ");
  Serial.println(LWiFi.gatewayIP());

  // print the received signal strength:
  long rssi = LWiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void webserver_loop(){

   LWiFiClient client = server.available();

    if(settings_modified){
    setting_save();
    settings_modified=false;
  }
  
  if (client) {
  Serial.println(F("new client"));
  readString = "";             
  
  long webpagetimeout=millis();
  while (client.connected()) {
   
    if((millis()-webpagetimeout)>5000){
      Serial.println("WEBSERVER TIMEOUT");
      break;  //up to 10 sec to read;
    }
    
    if (client.available()) {
      char c = client.read();
      Serial.write(c);
      // store character received in receive string
      if (readString.length() < 256) {
        readString += (c);
      }
    // check for end of line
    if (c == '\n') {
        // process line if its the "GET" request
        int strindex;
        if ((strindex=readString.indexOf("device_id=")) != -1) {
          settings_modified=true;
          String readdata= readString.substring(strindex+10,readString.indexOf("&",strindex));
          strindex=readString.indexOf("&",strindex);
          Serial.print("SET_DEVICE_ID:");
          Serial.println(readdata);
          readdata.toCharArray(setting.device_id,32);
          Serial.println(setting.device_id);
        }
        
        
        if ((strindex=readString.indexOf("ssid=",strindex)) != -1) {
          settings_modified=true;
          String readdata= readString.substring(strindex+5,readString.indexOf("&",strindex));
          strindex=readString.indexOf("&",strindex);
          Serial.print("SET_SSID:");
          Serial.println(readdata);
          readdata.toCharArray(setting.wifi_ssid,32);
          Serial.println(setting.wifi_ssid);
        }
        
        if ((strindex=readString.indexOf("security=",strindex)) != -1) {
          settings_modified=true;
          String readdata= readString.substring(strindex+9,readString.indexOf("&",strindex));
          strindex=readString.indexOf("&",strindex);
          Serial.print("SET_SECURITY:");
          Serial.println(readdata);
          setting.wifi_auth=readdata.toInt();
          if(setting.wifi_auth==0){
            setting.wifi_pass=="";
          }
        }
        
        if ((strindex=readString.indexOf("password=",strindex)) != -1) {
          settings_modified=true;
          String readdata= readString.substring(strindex+9,readString.indexOf(" ",strindex));
          Serial.print("SET_PASSWORD:");
          Serial.println(readdata);
          readdata.toCharArray(setting.wifi_pass,11);
          Serial.println(setting.wifi_pass);
        }
        

        
       // if a blank line was received (just cr lf, length of 2), then its the end of the request
       if (readString.length() == 2) {
         // add other commands here
         // send web page back to client 

         Serial.println(F("sending web page"));
         SendWebPage(client); 
         Serial.println(F("web page sent"));
         // break out and disconnect. This will tell the browser the request is complete without   having to specify content-length
         break;
       }  // end of request reached

       // start line over            
       readString = "";
     }  // end of line reached  
    }  // end data is available from client
   }  // end cient is connected
   // give the web browser time to receive the data
   Serial.println(F("delay before disconnect"));
   delay(1);
   // close the connection:
   client.stop();
   Serial.println(F("client disonnected")); 
   } else {
    printwebWifiStatus();
    delay(500);
 }
}

//----- setup -----

void setup() {
  Serial.begin(SERIAL_BAUDRATE);
  Serial.println("System starting...."); 
  
  
  //pinMode(BUZZER_ALARM_PIN, INPUT);
  
  
  // General
  sensor_setup();
  alarm_setup();
  
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

  //Web Setup Server
  boolean isHigh=false;
  for(int i=0;i<=5;i++){
     if(analogRead(A2)>500) isHigh=true;
     Serial.println(isHigh);
     delay(10);
  }
  
  //if((!setting_load()) || isHigh){
  if(1){
    Serial.println("Entering WIFI Setup Section...");
    setting_init();
    Serial.println("Default Setting loaded...");
    setting_save();
    Serial.println("Settings saved...");
    LWiFiEncryption wifi_auth;
    switch(setting.wifi_auth){
      case 0:
        wifi_auth=LWIFI_OPEN;
        memset(setting.wifi_pass, 0, sizeof(setting.wifi_pass));
      break;
      case 1:
        wifi_auth=LWIFI_WEP;
      break;
      case 2:
        wifi_auth=LWIFI_WPA;
      break;
    }
    
    String wifi_pass=setting.wifi_pass;
    wifi_pass.trim();
    Serial.print("Begin to connect: ");
    Serial.print(setting.wifi_ssid);
    Serial.print(" @Pass: ");
    Serial.println(wifi_pass);
    
    while (0 == LWiFi.connect(setting.wifi_ssid, LWiFiLoginInfo(wifi_auth, wifi_pass)))
      {
      Serial.println("WebPage DefaultMode");
      }
    server.begin();
    //while(1){  
      webserver_loop();
    //}
  }
  
   if(LED_MODE != LED_MODE_OFF){ // LED_MODE_OFF never light on  
    pinMode(ARDUINO_LED_PIN, OUTPUT);
  }
  
  lightBlink();
  setting_show();
  clientIDStr = setting.device_id;
  clientIDStr.toCharArray(clientID, clientIDStr.length()+1);

  
  display_current_setting(); 
  
  //wifiConnecting();
  //wifiConnected();  
  init_sensor_data();

  if(FAKE_GPS ==1){
    Serial.println("Using Fake GPS....Connect to Wifi now....");
    LWiFiEncryption wifi_auth;
    switch(setting.wifi_auth){
      case 0:
        wifi_auth=LWIFI_OPEN;
        memset(setting.wifi_pass, 0, sizeof(setting.wifi_pass));
      break;
      case 1:
        wifi_auth=LWIFI_WEP;
      break;
      case 2:
        wifi_auth=LWIFI_WPA;
      break;
    }
    String wifi_pass=setting.wifi_pass;
    wifi_pass.trim();
    while (0 == LWiFi.connect(setting.wifi_ssid, LWiFiLoginInfo(wifi_auth, wifi_pass)))
      {
      Serial.println("Connect WIFI");
      delay(1000);
      }
    Serial.println("Wifi Connected...send NTP info now");
    Udp.begin(2390);
    retrieveNtpTime();
    Serial.println("NTP time retrieved...");
    Udp.stop();
  }
  
  delay(3000);
  Serial.println("Setup complete! Looping main program");
  
}





//----- loop -----
void loop() {
  currentTime = millis();
  Serial.print("\n-----Loop ID: ");
  Serial.print(record_id);
  Serial.print(", current tick= ");
  Serial.print(currentTime);
  
  Serial.println(" -----");

  if(logic_select(LOGIC_WIFI_NEED_CONNECT)){
    wifiConnecting();
    wifiConnected(); 
  }
  
  // GPS  
  if(!FAKE_GPS){
    LGPS.getData(&info);
  }
  packInfo(INFO_GPS);
  // Sensor
  get_sensor_data();
  
  // Self alarm
  //alarm_self_handler();
  
  // MQTT
  packInfo(INFO_MQTT);  

  unsigned int need_send = logic_select(LOGIC_MQTT_NEED_SEND);
  unsigned int need_save;
  boolean bConnected;
  need_save=0;
  
  if(need_send){
 
     if(checkWifiConnected()==0){
       Serial.println("checkWifiConnected Fail!");
       need_save=1;
       wifi_ready=0;
     }
     else{   
    
        if (!mqttClient.connected()) {
          Serial.println("Reconnecting to MQTT Proxy");
          
          bConnected = mqttClient.connect(clientID);
          if(bConnected==false){
            Serial.println("Reconnecting to MQTT Proxy: Fail!");
            need_save=1;
          }
          mqttSubscribeRoutine();
        }
          mqttPrintCurrentMsg();
          mqttPublishRoutine(1);      
            //mqttClient.disconnect();
          LastPostTime = currentTime;
  
        // example:
        // Sensors/DustSensor |device_id=LASD-wuulong|time=20645|device=LinkItONE|values=0|gps=$GPGGA,235959.000,2448.0338,N,12059.5733,E,0,0,,160.1,M,15.0,M,,*4F
        
        
        if(LWiFi.status()!=LWIFI_STATUS_CONNECTED){
          wifi_ready=0;
          need_save=1;
          Serial.println("Wifi check fail!");
        }else{
          if(! mqttClient.connected()){
            wifi_ready=0;
            need_save=1;
            Serial.println("MQTT send fail!");
          }
        }
     }
  
  }
  

  if(logic_select(LOGIC_DATA_NEED_SAVETOFLASH) || need_save==1){      
  // Offline log
  packInfo(INFO_LOGFILE);

    Serial.println("Saving to file");
    // open the file. note that only one file can be open at a time, so you have to close this one before opening another.
    LFile dataFile = Drv.open(LOG_FILENAME, FILE_WRITE);
  
    // if the file is available, write to it:
    if (dataFile) {
      Serial.println(dataString);
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


  lightLed();
    
  //Serial.print(":");
  int usedTime = millis() - currentTime;
  signed long delayTime = (period_target[current_power_policy][PERIOD_SENSING_IDX]*1000) - usedTime + DELAY_SYS_EARLY_WAKEUP_MS;
  
#if BLYNK_ENABLE==1
  while( delayTime > 0 ){
    blynk_loop1();
    delay(BLYNK_POOLING_TIME);
    delayTime-=BLYNK_POOLING_TIME;
  }
#else
  if( delayTime > 0 ){
    delay(delayTime);
  }

#endif
  record_id++;
}








