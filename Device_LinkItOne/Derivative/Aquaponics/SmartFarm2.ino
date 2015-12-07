
#include <math.h>
#include <LFlash.h>
#include <LSD.h>
#include <LStorage.h>
#include <LTask.h>
#include <LWiFi.h>
#include <LWiFiClient.h>
#include <LBattery.h>
#include <Servo.h>
#include <LWiFiServer.h>

#define WIFI_AP   "laisan86"        //WiFi AP名稱
#define WIFI_PASSWORD "2lgigoal"  //WiFi密碼
#define WIFI_AUTH LWIFI_WPA  // choose from LWIFI_OPEN, LWIFI_WPA, or LWIFI_WEP.
#define SITE_URL "api.thingspeak.com"  // note that "caveeducation" is part of the HTTP request rather than host name.

const int pinBuz = 3;
const int pinLight = A1;
const int B=4275;                 // B value of the thermistor
const int R0 = 100000;            // R0 = 100k
const int pinTempSensor = A0;     // Grove - Temperature Sensor connect to A0
const int pinWater = A2;
const int pinRelay = 2;
const int pinServo = 9;
Servo feed;
LWiFiClient client;
int   sec = 0;
int   cnt30 = 0;
LWiFiServer server(80);

int   waterLow = 300;
int   secondsUpload = 30;
int   feedcnt = 3;        //960;
int   photolimit = 100;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while( !Serial ) delay(1500);

  pinMode(pinLight, INPUT);
  pinMode(pinTempSensor, INPUT);
  pinMode(pinWater, INPUT);
  pinMode(pinBuz, OUTPUT);
  pinMode(pinRelay, OUTPUT);

  digitalWrite(pinBuz, LOW);
  digitalWrite(pinRelay, LOW);

  LTask.begin();
  LWiFi.begin();
  Serial.println("Connecting to AP");
  while (0 == LWiFi.connect(WIFI_AP, LWiFiLoginInfo(WIFI_AUTH, WIFI_PASSWORD)))
  {
    delay(1000);
  }
  printWifiStatus();

  Serial.println("Start Web Server");
  server.begin();

  if(LSD.begin())       // 初始化
  {
    Serial.println("SD ok");
    
    
  } else {
    Serial.println("SD fail");
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  LWiFiClient smartphone = server.available();

  if (smartphone)
  {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (smartphone.connected())
    {
      if (smartphone.available())
      {
        // we basically ignores client request, but wait for HTTP request end
        int c = smartphone.read();
        Serial.print((char)c);

        if (c == '\n' && currentLineIsBlank)
        {
          Serial.println("send response");
          // send a standard http response header
          smartphone.println("HTTP/1.1 200 OK");
          smartphone.println("Content-Type: text/html");
          smartphone.println("Connection: close");  // the connection will be closed after completion of the response
          smartphone.println("Refresh: 5");  // refresh the page automatically every 5 sec
          smartphone.println();
          smartphone.println("<!DOCTYPE HTML>");
          smartphone.println("<html>");
          smartphone.println("<body><h1>Hello!</h1></body>");
          smartphone.println("</html>");
          smartphone.println();
          break;
        }
        if (c == '\n')
        {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r')
        {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(500);

    // close the connection:
    Serial.println("close connection");
    client.stop();
    Serial.println("client disconnected");
  }

  int value = analogRead(pinLight);
  value = map(value, 0, 1023, 255, 0);

  int a = analogRead(pinTempSensor);
  float R = log(((10240000/a)-10000));
  R = 1/(0.001129148 + (0.000234125 + (0.0000000876741 * R * R)) * R);
  float temperature=R-273.15;//convert to temperature via datasheet ;

  int water = analogRead(pinWater);

  if( value < photolimit )
  {
    digitalWrite(pinBuz, LOW);
  } else {
    digitalWrite(pinBuz, HIGH);
  }
  if( water > waterLow )
  {
    digitalWrite(pinRelay, LOW);
  } else {
    digitalWrite(pinRelay, HIGH);
  }

  int batt = LBattery.level();

  delay(1000);
  sec++;

  Serial.print("Photo: ");
  Serial.println(value, DEC);
  Serial.print("Temperature: ");
  Serial.printf("%2.2f\n", temperature);
  Serial.print("Water level: ");
  Serial.println(water, DEC);
  Serial.print("Battery(%): ");
  Serial.println(batt, DEC);

  if( sec >= secondsUpload )
  {
    Serial.println("Connecting to WebSite");
    while (client.connect(SITE_URL, 80) == 0)
    {
      Serial.println("Re-Connecting to WebSite");
      delay(1000);
    }
    Serial.println("Website connected!");
    client.print("GET /update");
    client.print("?key=E1ZKELVHGK4XYIHK&field1=");
    client.printf("%d", value);
    client.print("&field2=");
    client.printf("%2.2f", temperature);
    client.print("&field3=");
    client.printf("%d", water);
    client.print("&field4=");
    client.printf("%d", batt);
    client.println(" HTTP/1.1");
    client.println("Host: api.thingspeak.com");
    client.println("User-Agent: mtk-linkitone");
    client.println("Connection: close");
    client.println();
    
    sec = 0;
    cnt30++;
  }
  
  if( cnt30 >= feedcnt )
  {
    cnt30 = 0;
    feed.attach(pinServo);
    feed.write(179);
    delay(2000);
    feed.write(0);
    delay(30);
    feed.detach();
  }
}

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
