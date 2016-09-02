/*
	Version: 1.00
	Purpose: This project use LinkIt Smart 7688 Duo, G5(PMS5003), BME280, SHT25 and Gemtek LoRa module, 
             to achieve environmental sensing, target include PM1, PM2.5, PM10, temperature, humidity, embient pressure.
			 Using SeeedOLED for value display.
             
             Using sensors:
             	-- BME280: 		for temperature, humidity and ambient pressure
             	-- SHT25:		for temperature and humidity (for BME value calibration)
             	-- LoRa module: GIoT GL6509 (Gemtek LoRa)
             First, make sure all the sensors and lora module are working properly 
             in this project - G5, BME280, GIoT LoRa module and SHT25.
             Due to the bandwidth limitation when using Gemtek module, in Taipei, Taiwan,
             we need some bitwise operation, shifting everything into 11 bytes, the function "LoRaBitMap"
             below will help you in doing this. 
             For more information about bit shifting, please refer: http://www.slideshare.net/HuChengLee/taipei-iot-lora-workshop
    
    
	History:
	1.00  by Hu-Cheng Lee (Jack, jack77121@gmail.com)  01/09/2016 (DD/MM/YYYY) 
*/

//include library what we need
#include <SHT2x.h>
#include <SeeedOLED.h>
#include <SoftwareSerial.h>
#include <Adafruit_BME280.h>

//define sea level embient pressure
#define SEALEVELPRESSURE_HPA (1013.25)

//our bme280, using i2c
Adafruit_BME280 bme;

//our g5 sensor, using software serial
SoftwareSerial g5(11, 12); // RX, TX
#define pmsDataLen 32	//fix data length 32 bytes
uint8_t serialBuf[pmsDataLen];

//our LoRa module, using software serial
SoftwareSerial lora(5, 6); // RX, TX
byte lora_trans[11];	//lora buffer

//global variable for sensing target
float g_sht_temperature = 0;
float g_sht_humidity = 0;

float g_bme_temperature = 0;
float g_bme_humidity = 0;
float g_bme_baro = 0;

byte g_app_id = 5;	//5 for using LinkIt Smart 7688 Duo

int g_pm10 = 0;	//PM1
int g_pm25 = 0;	//PM2.5
int g_pm100 = 0;//PM10
int g_fix_num = 15;//value 15 means fake GPS, change this value to 0 if you using real GPS module

//GPS in DMS format
char GPS_LAT[] = "25.0228";  // device's gps latitude, IIS NRL, Academia Sinica
char GPS_LON[] = "121.3652"; // device's gps longitude, IIS NRL, Academia Sinica
//DD format: 25.041114, 121.614444


void setup()
{	

	Serial.begin(9600);	//debug serial
	bme.begin();		//BME280
	g5.begin(9600);		//G5
	lora.begin(9600);	//LoRa

	/*
		pin D10 and D13 are connected to G5 pin 6 and pin 3 (D10 <-> G5 pin6, D13 <-> G5 pin3), 
		set these to INPUT mode for temporarily, we might use this 2 pin in the future for power saving mode
	*/ 
	pinMode(10, INPUT);
	pinMode(13, INPUT); 

	//SeeedOled setting
	SeeedOled.init();  //initialze SEEED OLED display
	DDRB|=0x21;        //digital pin 8, LED glow indicates Film properly Connected .
	PORTB |= 0x21;
	SeeedOled.clearDisplay();          //clear the screen and set start position to top left corner
	SeeedOled.setNormalDisplay();      //Set display to normal mode (i.e non-inverse mode)
	SeeedOled.setPageMode();           //Set addressing mode to Page Mode
	delay(100); 
}

void loop()
{ 
	//get sensor value
	RetrieveG5Value();
	RetrieveSHTValue();
	RetrieveBMEValue();
	
		
	//for Oled display layout
	SeeedOled.setTextXY(0,0);          //Set the cursor to Xth Page, Yth Column
	SeeedOled.putString("Temp: "); //Print the String
	SeeedOled.putFloat(g_sht_temperature); 
	
	SeeedOled.setTextXY(1,0);          //Set the cursor to Xth Page, Yth Column
	SeeedOled.putString("Humi: "); //Print the String
	SeeedOled.putFloat(g_sht_humidity);

	
	SeeedOled.setTextXY(2,0);          //Set the cursor to Xth Page, Yth Column
	SeeedOled.putString("=====BME====="); //Print the String
	SeeedOled.setTextXY(3,0);          //Set the cursor to Xth Page, Yth Column
	SeeedOled.putString("Temp: "); //Print the String
	SeeedOled.putFloat(g_bme_temperature); 
	SeeedOled.setTextXY(4,0);          //Set the cursor to Xth Page, Yth Column
	SeeedOled.putString("Humi: "); //Print the String
	SeeedOled.putFloat(g_bme_humidity);
	SeeedOled.setTextXY(5,0);          //Set the cursor to Xth Page, Yth Column
	SeeedOled.putString("Baro: "); //Print the String
	SeeedOled.putFloat(g_bme_baro);

	SeeedOled.setTextXY(6,0);          //Set the cursor to Xth Page, Yth Column
	SeeedOled.putString("PM2.5: "); //Print the String
	SeeedOled.putNumber(g_pm25);
	SeeedOled.putString("ug/m3"); //Print the String
	SeeedOled.setTextXY(7,0);          //Set the cursor to Xth Page, Yth Column
	SeeedOled.putString("PM10: "); //Print the String
	SeeedOled.putNumber(g_pm100);
	SeeedOled.putString("ug/m3"); //Print the String
	lora.listen();
	delay(10000);
	//Do the bit shifting, and push out the data through LoRa module
	LoRaBitMap(g_app_id, g_bme_temperature, g_bme_humidity, g_pm25, g_pm100, GPS_LAT, GPS_LON, g_fix_num);

	//Show value in debug serial
	Display();
}
void RetrieveG5Value() {
	g5.listen();
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

	g_pm10 = ( serialBuf[10] << 8 ) | serialBuf[11];
	g_pm25 = ( serialBuf[12] << 8 ) | serialBuf[13];
	g_pm100 = ( serialBuf[14] << 8 ) | serialBuf[15];
}

void RetrieveSHTValue(){
	g_sht_temperature = SHT2x.GetTemperature();
	g_sht_humidity = SHT2x.GetHumidity();
}

void RetrieveBMEValue(){
	g_bme_temperature = bme.readTemperature();
	g_bme_humidity = bme.readHumidity();
	g_bme_baro = bme.readPressure() / 100.0F;
}

void Display(){
	Serial.print("BME Temperature:\t");
	Serial.println(g_bme_temperature);

	Serial.print("BME Humidity:\t\t");
	Serial.println(g_bme_humidity);

	Serial.print("BME Barometer:\t\t");
	Serial.println(g_bme_baro);

	Serial.print("SHT25 Temperature:\t");
	Serial.println(g_sht_temperature);

	Serial.print("SHT25 Humidity:\t\t");
	Serial.println(g_sht_humidity);

	Serial.print("G5 PM2.5:\t\t");
	Serial.println(g_pm25);

	Serial.print("G5 PM10:\t\t");
	Serial.println(g_pm100);
}

void LoRaBitMap(byte &app_id, float &temperature, float &humidity, int &pm25, int &pm100, char *GPS_LAT, char *GPS_LON, int &fix_num){
	word temperatureLora = (int)((temperature+20)*10);
	word humiditylora = (int)(humidity*10);
	word pm25lora = pm25;
	byte pm100Offset = pm100 - pm25;
	float GPS_LAT_f = (float)atof(GPS_LAT);
	float GPS_LON_f = (float)atof(GPS_LON);
//	Serial.println("=====================");
//	Serial.println(GPS_LAT);
//	Serial.println(GPS_LON);
//	Serial.println(GPS_LAT_f,4);
//	Serial.println(GPS_LON_f,4);
	
	
	GPS_LAT_f += 90;
	GPS_LON_f += 180;
//	Serial.println(GPS_LAT_f);
//	Serial.println(GPS_LON_f);
	unsigned long GPS_LAT_i = GPS_LAT_f*10000;
	unsigned long GPS_LON_i = GPS_LON_f*10000;	
//	Serial.println(GPS_LAT_i);
//	Serial.println(GPS_LON_i);
	
	byte lat_D = (int) GPS_LAT_f;
	float temp_lat_M = (GPS_LAT_f - lat_D)*100;
	byte lat_M = (int) temp_lat_M;
	byte lat_S = GPS_LAT_i%100;
	
	
	word lon_D = (int) GPS_LON_f;
	float temp_lon_M = (GPS_LON_f - lon_D)*100;
	byte lon_M = (int) temp_lon_M;
	byte lon_S = GPS_LON_i%100;
	
	byte gps_fix = fix_num;


	
	lora_trans[0] = (app_id << 4) | (temperatureLora >> 6);	
	lora_trans[1] = (temperatureLora << 2) | (humiditylora >> 8);
	lora_trans[2] = humiditylora;

	Serial.print(lora_trans[0],HEX);
	Serial.print("\t");
	Serial.print(lora_trans[1],HEX);
	Serial.print("\t");
	Serial.print(lora_trans[2],HEX);
	Serial.print("\t");
	// END FOR THE APP_ID, TEMPERATURE AND HUMIDITY

	

	lora_trans[3] =  pm25lora >> 3;
	lora_trans[4] = (pm25lora << 5)|(pm100Offset >> 3);
	
	if(lora_trans[3] < 16){
		Serial.print('0');
	}
	Serial.print(lora_trans[3],HEX);
	Serial.print("\t");
	if(lora_trans[4] < 16){
		Serial.print('0');
	}
	Serial.print(lora_trans[4],HEX);
	Serial.print("\t");
	//END FOR PM2.5 AND PM10	
	
	lora_trans[5] = (pm100Offset <<5) | (lat_D >> 3);
	lora_trans[6] = (lat_D << 5) | (lat_M >> 1);
	lora_trans[7] = (lat_M << 7) | (lat_S << 1) | (lon_D >> 8);
	lora_trans[8] = (byte)lon_D;
	if(lora_trans[5] < 16){
		Serial.print('0');
	}
	Serial.print(lora_trans[5],HEX);
	Serial.print("\t");
	Serial.print(lora_trans[6],HEX);
	Serial.print("\t");
	Serial.print(lora_trans[7],HEX);
	Serial.print("\t");
	Serial.print(lora_trans[8],HEX);
	Serial.print("\t");

	lora_trans[9] = (lon_M << 2) | (lon_S >> 4);
	lora_trans[10] = (lon_S << 4) | gps_fix;

	Serial.print(lora_trans[9],HEX);
	Serial.print("\t");
	Serial.println(lora_trans[10],HEX);

	char buff[150];
	
	sprintf(buff, "AT+DTX=22,%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\r\n", lora_trans[0], lora_trans[1], \
	lora_trans[2], lora_trans[3], lora_trans[4], lora_trans[5], lora_trans[6], lora_trans[7], lora_trans[8],  \
	lora_trans[9], lora_trans[10]);
	Serial.println(buff);
	lora.listen();
	lora.print(buff);
	
}
