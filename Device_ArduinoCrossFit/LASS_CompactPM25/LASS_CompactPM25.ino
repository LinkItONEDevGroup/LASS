/*
  LASS_CompactVersion for PM25_Field_Try
  This example demonstrate how to upload sensor data to MQTT server of LASS.
  It include features:
      (0) Choose Your Target Board
      (1) Connect to WiFi
      (2) Retrieve NTP time with Udp
      (3) Get PM 2.5 value from PMS3003 air condition sensor with UART
      (4) Optional DHT support ,comment #define USE_DHT
      (4) Connect to MQTT server and try reconnect when disconnect

      Oringinal By Realtek's Ameba release On LASS-Github, Strongly Modified By RODODO mini-techno-farm  see http://www.facebook.com/rododo.farm
      Support: Realtek Ameba ,ESP8266(NodeMCU 1.0), Arduino ATMEGA328P (not recommend), Leonardo , Mega , with Ethernet or Wifi Shield,
      Please Dont use ESP8266: Because LASS in support of TAIWAN IOT industry.
      TODO:Use intenal Flash on Nodemcu to Record when wifi loss
      WARNING:you should the PubSubClient.h on LASS-github to walk-through 128byte Limit
*/





char verapp[]="0.0.3";
//Please Choose your platform Here
#define BOARD_AMEBA
//#define BOARD_NODEMCU
//#define BOARD_ARDUINO


#if defined(BOARD_AMEBA) || defined(BOARD_NODEMCU)
  #ifdef BOARD_AMEBA
    #include <WiFi.h>
    char devicetype[] = "Ameba";
    #include <SoftwareSerial.h>;
    SoftwareSerial Serial1(0,1);
  #endif
  #ifdef BOARD_NODEMCU
    #include <ESP8266WiFi.h>
    char devicetype[] = "NodeMCU";
    #define Serial1 Serial
  #endif
  #include <WiFiUdp.h>
  WiFiClient netClient;
  WiFiUDP Udp;  
  char ssid[] = "Microwind_TL";      // your network SSID (name)
  char pass[] = "0919734011";     // your network password
  //int keyIndex = 0;               // your network key Index number (needed only for WEP)
#endif

#ifdef BOARD_ARDUINO
//Modify Here To Fit your Shield that comptiable with Ethernet.h
  #include <SPI.h>
  #include <Ethernet.h>
  #include <EthernetUdp.h>
  char devicetype[] = "Arduino";
  EthernetUDP Udp;
  EthernetClient netClient;
  // Enter a MAC address for your controller below.
  // Newer Ethernet shields have a MAC address printed on a sticker on the shield
  byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
  IPAddress ip(192, 168, 1, 177);
#endif

//From Configuration.h
char* ntpServerName = "time.nist.gov";
const char server[]= "gpssensor.ddns.net";
const char gps_lat[]= "23.711068";  // device's gps latitude
const char gps_lon[]= "120.545780"; // device's gps longitude
const char gps_alt[]= "30";         // device's gps altitude
//#define USE_AMEBA_DYNAMIC_ID ,if you use this funtion , clientid will be dynamic by MAC address
char clientId[] ="FT1_777";
char outTopic[] ="LASS/Test/PM25";
#define SENDLOOPTIME 60000 //ms //Upload data interval

//#error PLEASE SELECT SENSOR AND MARK THIS LINE with //
#define USE_PM25_G3
//#define USE_PM25_A4
//define USE_PM25_G5
#define USE_DHT
//#define USE_SHT31

#ifdef USE_DHT
  #include <DHT.h>
  #define DHTPIN 2     // what digital pin we're connected to
  #define DHTTYPE DHT22   // DHT11 DHT21 DHT22=(AM2302), AM2321
  DHT dht(DHTPIN, DHTTYPE);
#endif

#ifdef USE_SHT31
    #include <Wire.h>
    #include "sht3x.h"
    SHT3X sht3x;
#endif

void MQTTcallback(char* topic, byte* payload, unsigned int length) {
  Serial.print(F("Message arrived ["));
  Serial.print(topic);
  Serial.print(F("] "));
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

IPAddress timeServerIP; // time.nist.gov NTP server address

#include <PubSubClient.h>
PubSubClient mqttclient((char*)server, 1883, MQTTcallback, netClient);

void initializeNET() {

#if defined(BOARD_AMEBA) || defined(BOARD_NODEMCU)
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(F("WiFi connected"));  
  Serial.println(F("IP address: "));
  Serial.println(WiFi.localIP());
  WiFi.hostByName(ntpServerName, timeServerIP); 
#endif

#ifdef BOARD_ARDUINO
  Ethernet.begin(mac, ip);
#endif

  // local port to listen for UDP packets
  Udp.begin(2390);
}

//NTP
unsigned int localPort = 2390;      // local port to listen for UDP packets
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
uint32_t epochSystem = 0; // timestamp of system boot up

#ifdef BOARD_ARDUINO
unsigned long sendNTPpacket(char* address)
#endif

#if defined(BOARD_AMEBA) || defined(BOARD_NODEMCU)
// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
#endif
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
#if defined(BOARD_AMEBA) || defined(BOARD_NODEMCU)
    sendNTPpacket(timeServerIP);
#endif
#ifdef BOARD_ARDUINO
    sendNTPpacket(ntpServerName);
#endif
    delay(1000);
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

//Sensor
int pm25 = -1;
int pm10 = -1;
float h=-1;
float t=-1;

void retrieveTempHumidValue(){
#ifdef USE_DHT
  h = dht.readHumidity();
  t = dht.readTemperature();
  if(isnan(h) || isnan(t)) { h=-1;t=-1;}
#endif
#ifdef USE_SHT31
      sht3x.readSample();
      t=sht3x.getTemperature();
      h=sht3x.getHumidity();
#endif
Serial.print("Temp=");
Serial.print(t);
Serial.print(" C Humid=");
Serial.print(h);
Serial.println("%");
}

void retrievePM25Value(){
#ifdef USE_PM25_G3
#ifdef BOARD_AMEBA
#endif
  unsigned long timeout = millis();
  byte count=0;
  byte incomeByte[24];
  boolean startcount=false;
  byte data;
  while (1){
    if((millis() -timeout) > 1500) {    
      Serial.println(F("[G3-ERROR-TIMEOUT]"));
      //#TODO:make device fail alarm message here
      pm25=-1;
      pm10=-1;
      return;
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
  unsigned int calcsum = 0; // BM
  unsigned int exptsum;
  for(int i = 0; i < 22; i++) {
    calcsum += (unsigned int)incomeByte[i];
  }
  exptsum = ((unsigned int)incomeByte[22] << 8) + (unsigned int)incomeByte[23];
  if(calcsum == exptsum) {
    pm25 = ((unsigned int)incomeByte[12] << 8) + (unsigned int)incomeByte[13];
    pm10 = ((unsigned int)incomeByte[14] << 8) + (unsigned int)incomeByte[15];
  } else {
    Serial.println(F("#[G3-ERROR-CHECKSUM]"));
    pm25 = -1;
    pm10 = -1;
  }
#endif
#ifdef USE_PM25_A4
  unsigned long timeout = millis();
  int count=0;
  byte incomeByte[32];
  boolean startcount=false;
  byte data;
  while (1){
    if((millis() -timeout) > 1500) {    
      Serial.println("[A4-ERROR-TIMEOUT]");
      //#TODO:make device fail alarm message here.
      pm25=-1;
      pm10=-1;
      return;
    }
    if(Serial1.available()){
      data=Serial1.read();
      Serial.print(data,HEX);
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
  Serial1.write('\n');
  unsigned int calcsum = 0; // BM
  unsigned int exptsum;
  for(int i = 0; i < 29; i++) {
    calcsum += (unsigned int)incomeByte[i];
  }
  
  exptsum = ((unsigned int)incomeByte[30] << 8) + (unsigned int)incomeByte[31];
  if(calcsum == exptsum) {
    pm25 = ((unsigned int)incomeByte[6] << 8) + (unsigned int)incomeByte[7];
    pm10 = ((unsigned int)incomeByte[8] << 8) + (unsigned int)incomeByte[9];
  } else {
    Serial.println("#[A4-ERROR-CHECKSUM]");
    pm25 = -1;
    pm10 = -1;
  }
#endif

#ifdef USE_PM25_G5
      unsigned long timeout = millis();
      int count=0;
      byte incomeByte[32];
      boolean startcount=false;
      byte data;
      while (1){
        if((millis() - timeout) > 1500) {    
          Serial.println("[G5-ERROR-TIMEOUT]");
          *pm25 = -1;
          *pm10 = -1;
          return;
        }
        if(Serial1.available()){
          data=Serial1.read();
          if(data==0x42 && !startcount){
            startcount = true;
            count++;
            incomeByte[0]=data;
          } else if (startcount){
            count++;
            incomeByte[count-1]=data;
            if(count>=32) {break;}
          }
        }
      }
      unsigned int calcsum = 0; // BM
      unsigned int exptsum;
      for(int i = 0; i < 29; i++) {
        calcsum += (unsigned int)incomeByte[i];
      }
    
      exptsum = ((unsigned int)incomeByte[30] << 8) + (unsigned int)incomeByte[31];
      if(calcsum == exptsum) {
        *pm25 = ((unsigned int)incomeByte[12] << 8) + (unsigned int)incomeByte[13];
        *pm10 = ((unsigned int)incomeByte[14] << 8) + (unsigned int)incomeByte[15];
      } else {
        Serial.println("#[exception] PM2.5 Sensor CHECKSUM ERROR!");
        *pm25 = -1;
        *pm10 = -1;
      }  
#endif

}

//MQTT

void initializeMQTT() {
  Serial.print(F("MQTT client id:"));
  Serial.println(clientId);
  Serial.print(F("MQTT topic:"));
  Serial.println(outTopic);
}

void reconnectMQTT() {
  // Loop until we're reconnected
  while (!mqttclient.connected()) {
    Serial.print(F("Attempting MQTT connection..."));
    // Attempt to connect
    if (mqttclient.connect(clientId)) {
      Serial.println(F("connected"));
      mqttclient.subscribe(outTopic);
    } else {
      Serial.println(F("Failed... try again in 5 seconds"));
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void sendMQTTMessage(){
  char payload[300];
  unsigned long epoch = epochSystem + millis() / 1000;
  int year, month, day, hour, minute, second;
  getCurrentTime(epoch, &year, &month, &day, &hour, &minute, &second);



#if defined(USE_DHT)||defined(USE_SHT31)
  int t_s = (int)t;
  int t_sf= ((int)(t*10))%10;
  int h_s = (int)h;
    sprintf(payload, "|ver_format=3|Fake_GPS=1|app=PM25|ver_app=%s|device_id=%s|tick=%d|date=%4d-%02d-%02d|time=%02d:%02d:%02d|device=%s|s_d0=%d.00|s_t0=%d.%d|s_h0=%d|s_d1=%d.00|gps_lat=%s|gps_lon=%s|gps_fix=1|gps_alt=%s",
    verapp,clientId,
    millis(),
    year, month, day,
    hour, minute, second,
    devicetype,pm25,t_s,t_sf,h_s,pm10,
    gps_lat, gps_lon,gps_alt
    );
#else
  sprintf(payload, "|ver_format=3|Fake_GPS=1|app=PM25|ver_app=%s|device_id=%s|tick=%d|date=%4d-%02d-%02d|time=%02d:%02d:%02d|device=%s|s_d0=%d.00|s_d1=%d.00|gps_lat=%s|gps_lon=%s|gps_fix=1|gps_alt=%s",
  verapp,clientId,
  millis(),
  year, month, day,
  hour, minute, second,
  devicetype,pm25,pm10,
  gps_lat, gps_lon,gps_alt
  );
#endif
  // Once connected, publish an announcement...
  char companionchannel[32]="";
  strcat(companionchannel,outTopic);
  strcat(companionchannel,"/");
  strcat(companionchannel,clientId);
  mqttclient.publish((char*)outTopic,payload);
  mqttclient.publish((char*)companionchannel,payload);
  Serial.print(outTopic);
  Serial.println(payload);
  Serial.print(companionchannel);
  Serial.println(payload);
}

void sensorInit(){
#ifdef USE_DHT
  dht.begin();
#endif
#ifdef USE_SHT31
  sht3x.setAddress(SHT3X::I2C_ADDRESS_44);
  sht3x.setAccuracy(SHT3X::ACCURACY_HIGH);
  Wire.begin(); 
#endif
#ifdef BOARD_AMEBA
  #ifdef USE_AMEBA_DYNAMIC_ID
    byte mac[6];
    WiFi.macAddress(mac);
    memset(clientId, 0, MAX_CLIENT_ID_LEN);
    sprintf(clientId, "FT1_0%02X%02X", mac[4], mac[5]);
  #endif
#endif
}

void setup()
{
  Serial1.begin(9600); // PMS 3003 UART has baud rate 9600
  #ifdef LASS_DEBUG
    Serial.begin(9600);
  #endif
  delay(10);
  initializeNET();
  retrieveNtpTime();
  initializeMQTT();
  sensorInit();
  // Allow the hardware to sort itself out
  delay(1500);
}

long lastMsg = 0;
boolean firstmove = ture;
long loopcount;
void loop()
{ 
  Serial.print("loopcount:");
  Serial.println(loopcount);
  loopcount++;
  //Process Filter or any logic control below
  long now = millis();
  if(mqttclient.connected()){
    if (now - lastMsg > SENDLOOPTIME) {
      retrieveTempHumidValue();
      retrievePM25Value();
      lastMsg = now;
      sendMQTTMessage();
    } else if(firstmove) {
      lastMsg = now;
      sendMQTTMessage();
      firstmove = false; //run only once
    }
  } else {
    reconnectMQTT();
  }
  mqttclient.loop();
}


