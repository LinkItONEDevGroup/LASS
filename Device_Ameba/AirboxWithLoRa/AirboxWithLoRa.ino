/*
	Version: 1.00
	Purpose: This sample use Ameba and Gemtek LoRa module, 
             to build your own "Airbox" in Taipei.
             Using sensors:
             	-- PMS5003: 	for PM2.5, PM10 sensing
             	-- SHT31: 		for temperature and humidity
             	-- LoRa module: GIoT GL6509 (Gemtek LoRa)
             First, make sure all the sensors and lora module are working properly 
             in this project - PMS5003, SHT31 & GIoT LoRa module.
             Due to the bandwidth limitation when using Gemtek module, in Taipei, Taiwan,
             we need some bitwise operation, shifting everything into 11 bytes, the function "LoRaBitMap"
             below will help you in doing this.
    
    
	History:
	1.00  by Hu-Cheng Lee (Jack, jack77121@gmail.com)  17/05/2016 (DD/MM/YYYY) 
*/


#include <SoftwareSerial.h>
#include "Adafruit_SHT31.h"

#define pmsDataLen 32

Adafruit_SHT31 sht31 = Adafruit_SHT31();	//SHT31

SoftwareSerial g5(17, 5); // RX, TX
SoftwareSerial lora(0, 1); // RX, TX

uint8_t serialBuf[pmsDataLen];
int idx = 0;
int pm10 = 0;
int pm25 = 0;
int pm100 = 0;
int fix_num = 15;			// 15 for fake GPS, who don't have GPS module

byte app_id = 0;
byte lora_trans[11];

float temperature = 0;
float humidity = 0;
		 				

/*  
*	GPS example for IIS NRL, Academia Sinica in Taipei, Taiwan.
* 	Using DMS format: 
*	D-25 M-02 S-28 for latitude
*	D-121 M-36 S-52 for latitude
* 	Replace this position to your location whatever you are :)
*/
char gps_lat[] = "25.0228";  // device's gps latitude,  
char gps_lon[] = "121.3652"; // device's gps longitude, IIS NRL, Academia Sinica




void setup() {
  // put your setup code here, to run once:
	Serial.begin(9600);
	g5.begin(9600); 	// 	PMS 5003 UART has baud rate 9600
	lora.begin(9600);	//LoRa
	sht31.begin(0x44);	//SHT31 begin
	Serial.println("start");
	delay(1500);
}

void loop() {
  	// put your main code here, to run repeatedly:
 	retrievePM25Value();
 	retrieveSHT31Value();
	LoRaBitMap(app_id, temperature, humidity, pm25, pm100, gps_lat, gps_lon, fix_num);
	
	delay(60000);
}

void LoRaBitMap(byte &app_id, float &temperature, float &humidity, int &pm25, int &pm100, char *gps_lat, char *gps_lon, int &fix_num){
	word temperatureLora = (int)((temperature+20)*10);
	word humiditylora = (int)(humidity*10);
	word pm25lora = pm25;
	byte pm100Offset = pm100 - pm25;
	float gps_lat_f = (float)atof(gps_lat);
	float gps_lon_f = (float)atof(gps_lon);
	gps_lat_f += 90;
	gps_lon_f += 180;
	int gps_lat_i = (int) (gps_lat_f*10000);
	int gps_lon_i = (int) (gps_lon_f*10000);	
	byte lat_D = (int) gps_lat_f;
	float temp_lat_M = (gps_lat_f - lat_D)*100;
	byte lat_M = (int) temp_lat_M;
	byte lat_S = (int) gps_lat_i%100;
	word lon_D = (int) gps_lon_f;
	float temp_lon_M = (gps_lon_f - lon_D)*100;
	byte lon_M = (int) temp_lon_M;
	byte lon_S = (int) gps_lon_i%100;
	byte gps_fix = fix_num;
	char buff[150];
	
	lora_trans[0] = (app_id << 4) | (temperatureLora >> 6);	
	lora_trans[1] = (temperatureLora << 2) | (humiditylora >> 8);
	lora_trans[2] = humiditylora;
		// END FOR THE APP_ID, TEMPERATURE AND HUMIDITY
	lora_trans[3] =  pm25lora >> 3;
	lora_trans[4] = (pm25lora << 5)|(pm100Offset >> 3);
		// END FOR PM2.5	
	lora_trans[5] = (pm100Offset <<5) | (lat_D >> 3);
	lora_trans[6] = (lat_D << 5) | (lat_M >> 1);
	lora_trans[7] = (lat_M << 7) | (lat_S << 1) | (lon_D >> 8);
	lora_trans[8] = (byte)lon_D;
	lora_trans[9] = (lon_M << 2) | (lon_S >> 4);
	lora_trans[10] = (lon_S << 4) | gps_fix;
		// END FOR PM10 AND GPS
	sprintf(buff, "AT+DTX=22,%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\r\n", \
	lora_trans[0], lora_trans[1], lora_trans[2], lora_trans[3], lora_trans[4], \
	lora_trans[5], lora_trans[6], lora_trans[7], lora_trans[8], lora_trans[9], lora_trans[10]);
	lora.print(buff);
}
void retrievePM25Value() {
	
	uint8_t c = 0;
	int idx = 0;
	memset(serialBuf, 0, pmsDataLen);
	
	while (true) {
		while (c != 0x42) {
			while (!g5.available());
			c = g5.read();
		}
		while (!g5.available());
		c = g5.read();
		if (c == 0x4d) {
			// now we got a correct header)
			serialBuf[idx++] = 0x42;
			serialBuf[idx++] = 0x4d;
			break;
		}
	}

	while (idx != pmsDataLen) {
		while(!g5.available());
		serialBuf[idx++] = g5.read();
	}

	pm10 = ( serialBuf[10] << 8 ) | serialBuf[11];
	pm25 = ( serialBuf[12] << 8 ) | serialBuf[13];
	pm100 = ( serialBuf[14] << 8 ) | serialBuf[15];
}

void retrieveSHT31Value(){
	temperature = sht31.readTemperature();
	humidity = sht31.readHumidity();
}
