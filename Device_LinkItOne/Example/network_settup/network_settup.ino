#include <LTask.h>
#include <LWiFi.h>
#include <LWiFiServer.h>
#include <LWiFiClient.h>
#include "configuration.h"

LWiFiServer server(80);

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
  char gps_fix_infor[64];
  //other
  char blynk_auth[33];
  //reserved, prepare for some compatibility need
  char reserved[256];
} SETTING;

void printWifiStatus()
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

SETTING setting;
#define Drv LFlash          // use Internal 10M Flash
#include <LFlash.h>
#include <LSD.h>
#include <LStorage.h>
#define SETTING_FILENAME "setting.bin"

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
	
  strcpy(setting.gps_fix_infor, GPS_FIX_INFOR);
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
  Serial.print("GPS_FIX_INFOR=");
  Serial.println(setting.gps_fix_infor);
  
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
    connstr+="\" disabled=\"false\">\n";
  }
  connstr+="</td></tr>\n";
  connstr+="<tr><td>\n";
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


void setup()
{
  delay(5000);
  LTask.begin();
  LWiFi.begin();
  Serial.begin(115200);
  pinMode(INPUT_PULLUP,13);
  Serial.print("Initializing flash interface...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(STORAGE_CHIP_SELECT_PIN, OUTPUT);
  
  // see if the card is present and can be initialized:
  Drv.begin();
  if(!setting_load() || digitalRead(13) == 1){
    setting_init();
    setting_save();
  }
  pinMode(OUTPUT,13);  
  Serial.println("Connecting to AP");
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
    delay(1000);
  }
  server.begin();
}

String readString = String(100); //string for fetching data from address


void loop()
{
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
    printWifiStatus();
    Serial.print("WIFI:");
    Serial.println(setting.wifi_pass);
    delay(1000);
  }

}

