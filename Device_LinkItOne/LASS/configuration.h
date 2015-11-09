//LASS CONFIGURATION FILE 

//Step 1: How you Connect WIFI....Basic things.
//WIFI
//System default wifi setting: SSID=LASS, PASS=LASS123456, WIFI_AUTH=LWIFI_WPA
#define WIFI_SSID "LASS"         // REPLACE: your network SSID (name)
#define WIFI_PASS "LASS123456"   // REPLACE: your network password (use for WPA, or use as key for WEP)
#define WIFI_AUTH LWIFI_WPA   //Default:LWIFI_WPA // choose from LWIFI_OPEN, LWIFI_WPA, or LWIFI_WEP.
//-----------------------------------------------------------

//Step 2:Do you use Blyak.If yes,fill info below.
//Blynk-IoT
#define BLYNK_ENABLE 0 // deafult(0) 0: If you don't need to support BLYNK, 1: support BLYNK 
#if BLYNK_ENABLE==1
  char blynk_auth[] = "YourAuthToken"; // REPLACE: your Blynk auto id
#endif
//-----------------------------------------------------------

//Step 3:MQTT info

//MQTT-IoT
#define MQTT_PROXY_IP "gpssensor.ddns.net"  // Current LASD server , dont change!
#define DEVICE_TYPE  "LinkItONE"            // since there is only one device LASS supported now,dont change!
#define DEVICE_ID "FT1_999"                 // REPLACE: The device ID you like, please start from LASD. Without this prefix, maybe it will be filter out.
#define MQTT_TOPIC_PREFIX "LASS/Test"       // CAN REPLACE if you like
#define PARTNER_ID "LASS-Partner1"          // CAN REPLACE if you like

//Step 5:GPS
//Do you want to use gps? 0:YES 1:FAKE GPS
#define FMT_OPT 1 // FMT_OPT : 0: default format with gps, 1: default format but gps is fix data, need to update GPS_FIX_INFOR 
//NOTICE:If you choose 1 modify "FAKE" GPS location. below
#define GPS_FIX_INFOR "$GPGGA,064205.096,0,N,0,E,0,0,,207.8,M,15.0,M,,*4F\r" // If the device don't have GPS, setup the FIX GPS information here. The checksum don't need to be correct 
#define GPS_SIGNAL_NOCHECK 1   // 0: log or send only when GPS have signal, 1: always log and send even when GPS have no signal 



//NOTICE: for Field TRY-PM2.5 DONT CHANGE AFTER THIS LINE!   --2015/11/09
//-----------------------------------------------------------
//Step 6:About LASS 
#define APP_ID (APPTYPE_SYSTEM_BASE+1)               // REPLACE: this is your unique application 0-255: system reserved, 256-32767: user public use, 32768-65536: private purpose
#define APPTYPE_SYSTEM_BASE 0
#define APPTYPE_PUBLIC_BASE 256
#define APPTYPE_PRIVATE_BASE 32768
#define SERIAL_BAUDRATE 115200

//NOTICE: You are ready to ROCK, NO MORE TO SETUP!!!!
//-----------------------------------------------------------


//For ADVANCED user ONLY 
#define ALARM_ENABLE 1 // default(0) 0: disable alarm, 1: enable alarm

//----- USER SENSOR CONFIG -----

#define SENSOR_CNT 20          // REPLACE: the sensors count that publish to server.
#define SENSOR_STRING_MAX 300  

#define SENSOR_ID_RECORDID 0
#define SENSOR_ID_BATTERYLEVEL 1
#define SENSOR_ID_BATTERYCHARGING 2 //      battery is charging: (0) not charging, (1) charging
#define SENSOR_ID_GROUNDSPEED 3

//LASS's OPEN PM2.5 Field-TRY
#if APP_ID==(APPTYPE_SYSTEM_BASE+1)
  #define SENSOR_ID_DUST 10
  #define SENSOR_ID_TEMPERATURE 11
  #define SENSOR_ID_HUMIDITY 12  
  #define SENSOR_ID_DUST_BLYNK 4
  #define SENSOR_ID_TEMPERATURE_BLYNK 5
  #define SENSOR_ID_HUMIDITY_BLYNK 6

//LASS's PM2.5 project by RODODO-MINGWEI
#elif APP_ID==(APPTYPE_PUBLIC_BASE+2)
  #define SENSOR_ID_DUST 10  
  #define SENSOR_ID_DUST_BLYNK 4 

//LASS's start up project by wuloong
#elif APP_ID==(APPTYPE_PUBLIC_BASE+1)
  #define SENSOR_ID_DUST 10
  #define SENSOR_ID_UV 11
  #define SENSOR_ID_SOUND 12
  // in order to prevent blynk not support that many virtual gpio in the macro. we setup virtual GPIO in lower pin
  #define SENSOR_ID_DUST_BLYNK 4
  #define SENSOR_ID_UV_BLYNK 5
  #define SENSOR_ID_SOUND_BLYNK 6

//LASS project by Academia Sinica-LJ
#elif APP_ID==(APPTYPE_PUBLIC_BASE+3)
  #define SENSOR_ID_BAROMETER 10
  #define SENSOR_ID_TEMPERATURE 11
  #define SENSOR_ID_HUMIDITY 12  
  #define SENSOR_ID_LIGHT 13
  // in order to prevent blynk not support that many virtual gpio in the macro. we setup virtual GPIO in lower pin
  #define SENSOR_ID_BAROMETER_BLYNK 4
  #define SENSOR_ID_TEMPERATURE_BLYNK 5
  #define SENSOR_ID_HUMIDITY_BLYNK 6
  #define SENSOR_ID_LIGHT_BLYNK 7
#endif 





//SYSTEM PARAMETERS

#define DELAY_SYS_EARLY_WAKEUP_MS 11

#define POLICY_ONLINE_ALWAYS 1
#define POLICY_ONLINE_LESS 2
#define POLICY_ONLINE_DEFAULT 0

#define POLICY_POWER_DONTCARE 0
#define POLICY_POWER_SAVE 1
#define POLICY_POWER_AUTO 2

#define LED_MODE_DEFAULT 0 // to show system status and behavior
#define LED_MODE_OFF 1 // To not disturbe the environment, never have LED on 


#define PERIOD_SENSING_IDX 0
#define PERIOD_UPLOAD_IDX 1
#define PERIOD_WIFICHECK_IDX 2


#define POLICY_ONLINE POLICY_ONLINE_ALWAYS //1: POLICY_ONLINE_ALWAYS 2: POLICY_ONLINE_LESS                                            
#define POLICY_POWER  POLICY_POWER_DONTCARE //2: POLICY_POWER_AUTO(Auto power saving mode) 0: POLICY_POWER_DONTCARE 1: POLICY_POWER_SAVE
                                            // policy auto check if not charging and battery lower than seting of battery level, switch to power saving mode.

#define POWER_POLICY_BATTERY_LEVEL 70 // When battery level lower than this, trigger power saving mode when power policy is AUTO
#define LED_MODE LED_MODE_DEFAULT  


// The logic decide if we should do something
#define LOGIC_WIFI_NEED_CONNECT 1
#define LOGIC_MQTT_NEED_SEND 2
#define LOGIC_DATA_NEED_SAVETOFLASH 3
#define LOGIC_WHAT_LED_STATE 4
#define LOGIC_LOG_NEED_SEND 5

#define LED_STATE_OFF 0
#define LED_STATE_READY 1
#define LED_STATE_ERROR 2






