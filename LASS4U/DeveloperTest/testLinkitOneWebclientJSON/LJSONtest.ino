/*
  Web client

 This sketch connects to a website 
 using Wi-Fi functionality on MediaTek LinkIt platform.

 Change the macro WIFI_AP, WIFI_PASSWORD, WIFI_AUTH and SITE_URL accordingly.

 created 13 July 2010
 by dlf (Metodo2 srl)
 modified 31 May 2012
 by Tom Igoe
 modified 20 Aug 2014
 by MediaTek Inc.
 */

#include <LTask.h>
#include <LWiFi.h>
#include <LWiFiClient.h>
#include <ArduinoJson.h>
#define WIFI_AP "Microwind_TWN"
#define WIFI_PASSWORD "0919734011"
#define WIFI_AUTH LWIFI_WPA  // choose from LWIFI_OPEN, LWIFI_WPA, or LWIFI_WEP.
#define SITE_URL "nrl.iis.sinica.edu.tw"

LWiFiClient c;

void setup()
{
  LWiFi.begin();
  Serial.begin(115200);

  // keep retrying until connected to AP
  Serial.println("Connecting to AP");
  while (0 == LWiFi.connect(WIFI_AP, LWiFiLoginInfo(WIFI_AUTH, WIFI_PASSWORD)))
  {
    delay(1000);
  }

  // keep retrying until connected to website

}

boolean disconnectedMsg = false;

void loop()
{
  boolean jsonhead=0;
  String json;
  json.reserve(4096);
  // Make sure we are connected, and dump the response content to Serial
  Serial.println("Connecting to WebSite");
  while (0 == c.connect(SITE_URL, 80))
  {
    Serial.println("Re-Connecting to WebSite");
    delay(1000);
  }

  // send HTTP request, ends with 2 CR/LF
  Serial.println("send HTTP GET request");
  c.println("GET /LASS/history-hourly.php?device_id=FT1_001 HTTP/1.1");
  c.println("Host: " SITE_URL);
  c.println("Connection: close");
  c.println();

  // waiting for server response
  Serial.println("waiting HTTP response:");
  while (!c.available())
  {
    delay(100);
  }
  int jsoncount=0;
  while (c)
  {
    char v = c.read();
    if (v != -1)
    {
      Serial.print((char)v);
      if(v=='{') {jsonhead = 1;jsoncount++;}
      if(jsonhead ==1) {json +=v;}
    }
    else
    {
      Serial.println("no more content, disconnect");
      c.stop();
      while (1)
      {
        delay(1);
      }
    }
  }
 if (!disconnectedMsg)
  {
    Serial.println();
    Serial.println("disconnected by server");
    disconnectedMsg = true;
  }
  
  
  Serial.println();
  Serial.println(json);
  
  StaticJsonBuffer<4096> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
  
  // Test if parsing succeeds.
  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }
  
  byte c_data[jsoncount];
  int i=0;
  while(i<jsoncount-1){
    byte sensor =root["feeds"][i]["PM2_5"];
    c_data[i] = sensor;
    Serial.println(sensor);
    Serial.flush();
    i++;
  }
  
 delay(20000);
}

