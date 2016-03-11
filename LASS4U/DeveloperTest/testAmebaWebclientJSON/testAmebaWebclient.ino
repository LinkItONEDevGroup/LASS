
#include <WiFi.h>

#include <ArduinoJson.h>

char ssid[] = "Microwind_TWN"; //  your network SSID (name)
char pass[] = "0919734011";    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;
//IPAddress server(64,233,189,94);  // numeric IP for Google (no DNS)
char server[] = "nrl.iis.sinica.edu.tw";    // name address for Google (using DNS)

WiFiClient client;

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ;
  }
  // attempt to connect to Wifi network:
  
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(2000);
  }
  
  Serial.println("Connected to wifi");
  //printWifiStatus();

  Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
}

void loop() {
  while (0 == client.connect(server, 80))
  {
    Serial.println("Re-Connecting to WebSite");
    delay(1000);
  }
  
  //if (client.connect(server, 80)) {
    Serial.println("connected to server");
    // Make a HTTP request:
    client.println("GET /LASS/history-hourly.php?device_id=FT1_001 HTTP/1.1");
    client.println("Host: nrl.iis.sinica.edu.tw");
    client.println("Connection: close");
    client.println();
  //}
  // if there are incoming bytes available
  // from the server, read them and print them:
  boolean jsonhead=0;
  String json;
  json.reserve(4096);
  while (client.available()) {
    char c = client.read();
      if(c=='{') {jsonhead = 1;}
      if(jsonhead ==1) {json +=c;}
  }
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting from server.");
    client.stop();

    // do nothing forevermore:
    delay(10000);
  }

  Serial.println();
  Serial.print(json);
  
  StaticJsonBuffer<4096> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
  
  // Test if parsing succeeds.
  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }
  byte c_data[31];
  int i=0;
  while(i<31){
    byte sensor =root["feeds"][i]["PM2_5"];
    c_data[i] = sensor;
    Serial.println(sensor);
    Serial.flush();
    i++;
  }
  
  // if the server's disconnected, stop the client:

}

/*
void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
*/
