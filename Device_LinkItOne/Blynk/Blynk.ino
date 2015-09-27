/**************************************************************
 * Blynk is a platform with iOS and Android apps to control
 * Arduino, Raspberry Pi and the likes over the Internet.
 * You can easily build graphic interfaces for all your
 * projects by simply dragging and dropping widgets.
 *
 *   Downloads, docs, tutorials: http://www.blynk.cc
 *   Blynk community:            http://community.blynk.cc
 *   Social networks:            http://www.fb.com/blynkapp
 *                               http://twitter.com/blynk_app
 *
 * Blynk library is licensed under MIT license
 * This example code is in public domain.
 *
 **************************************************************
 *
 * This example shows how to use LinkItONE to connect your project to Blynk.
 *
 * Change WiFi ssid, pass, and Blynk auth token to run :)
 * Feel free to apply it to any other example. It's simple!
 *
 **************************************************************/
// This sample verified with the Blynk 3.1 and iphone. checked with Analog A0, Virtual V0 
#define ARDUINO 150 // to avoid Blynk library use yield() in function run(), without this. system will crash!

#include <LWiFi.h>
#include <LWiFiClient.h>
#include <BlynkSimpleLinkItONE.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "5cb1d7f458c041b0a195cf2515ef2d96";

#define WIFI_SSID "LASS"     // Your network SSID (name)
#define WIFI_PASS "LASS123456"     // Your network password (use for WPA, or use as key for WEP)
#define WIFI_AUTH LWIFI_WPA      // choose from LWIFI_OPEN, LWIFI_WPA, or LWIFI_WEP

void setup()
{
  Serial.begin(9600);
  Blynk.begin(auth, WIFI_SSID, WIFI_PASS, WIFI_AUTH);
  Serial.println("System start!");

}
uint16_t loop_id=0;
void loop()
{
  
  loop_id++;
  Serial.println(loop_id);
  Blynk.run();
}

BLYNK_READ(0) // sensorValue[0] : Sound
{
  Serial.println("Blynk comes to read!");
  Blynk.virtualWrite(0, loop_id);
}

