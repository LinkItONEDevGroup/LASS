/*
    Version: 1.00
    Purpose: This example use mbed NXP LPC1769 
             Using sensors:
                -- PMS3003: forPM1.0, PM2.5, PM10 sensing
                -- DHT22: for temperature and humidity
             Using transfer module:
                -- LoRa module: GIoT GL6509 (Gemtek LoRa)
             Circuit diagram remind:
                --DHT22: Data pin parallel to Vcc(use 5.1KΩ)
                --Grove_LCD_RGB_Backlight: Sda pin and Scl pin parallel to Vcc(use 5.1KΩ) 

*/

#include "mbed.h"
#include "DHT.h"
#include "Grove_LCD_RGB_Backlight.h"

Serial pc(USBTX, USBRX); // tx, rx
Serial device(p28, p27); //tx, rx
Serial lora(p13, p14); //tx, rx
DHT dht22(p21, SEN51035P);
Grove_LCD_RGB_Backlight rgbLCD(p9, p10);

char buffer[24];
int app_id = 15;
char lora_trans[11];
char gps_lat[] = "25.0227";
char gps_lon[] = "121.3653";
int fix_num = 15;
int count = 1;
int err ;
unsigned short pm1;
unsigned short pm25;
unsigned short pm10;

int main() {
    wait(2);
    device.baud(9600);
    lora.baud(9600);
    wait(1);
    buffer[0] = 0x42;
    buffer[1] = 0x4d;
    while (1) {
        while (device.getc() != 0x42) {
            }
            if (device.getc() == 0x4d) {
                for (int i = 2; i <24; i++) {
                    buffer[i] = device.getc();
                    }
                unsigned short calcsum = buffer[2] + buffer[3]+ buffer[4] + buffer[5] + buffer[6] + buffer[7] + buffer[8] + buffer[9] + buffer[10] + buffer[11] + buffer[12] + buffer[13] + buffer[14] + buffer[15] + buffer[16] + buffer[17] + buffer[18] + buffer[19] + buffer[20] + buffer[21];
                unsigned short calcsum1 = (calcsum + 0x42 +0x4d);
                unsigned short exptsum = (buffer[22]<<8) + buffer[23];
                if( exptsum == calcsum1){
                pm1 = buffer[10] + buffer[11];
                pm25 = buffer[12] + buffer[13];
                pm10 = buffer[14] + buffer[15];
                }
        }
        wait(1);
        do{
        err = dht22.readData();
        }while(err != 0);
        float temp = dht22.ReadTemperature(CELCIUS);
        float hum = dht22.ReadHumidity();
        wait(0.5);
            pc.printf("T: %.1f C \n", temp);
            pc.printf("H: %.1f %% \n", hum);
            pc.printf("pm1= %X ug/m3\n", pm1);
            pc.printf("pm25= %X ug/m3\n", pm25);
            pc.printf("pm10= %X ug/m3\n", pm10);
            pc.printf("%i \n" ,count);
            char lcd_h_buff[16];
            char lcd_buff[12];
            sprintf(lcd_h_buff, "T: %.1f H: %.1f" ,temp ,hum);
            sprintf(lcd_buff, "PM25:%i PM10:%i" ,pm25 ,pm10);
            rgbLCD.setRGB(0xff, 0xff, 0xff);
            rgbLCD.clear();
            rgbLCD.locate(0,0);
            rgbLCD.print(lcd_h_buff);
            rgbLCD.locate(0,1);
            rgbLCD.print(lcd_buff);
            wait(5);
            
        if(count==13&&temp!=0){
            int temperatureLora = (int)((temp + 20) * 10);
            int humiditylora = (int)(hum * 10);
            unsigned char pm25lora = pm25;
            unsigned char pm100Offset = pm10 - pm25;
            float gps_lat_f = (float) atof(gps_lat);
            float gps_lon_f = (float) atof(gps_lon);
            gps_lat_f += 90;
            gps_lon_f += 180;
            int gps_lat_i = (int)(gps_lat_f * 10000);
            int gps_lon_i = (int)(gps_lon_f * 10000);
            unsigned char lat_D = (int) gps_lat_f;
            float temp_lat_M = (gps_lat_f - lat_D) * 100;
            unsigned char lat_M = (int) temp_lat_M;
            unsigned char lat_S = (int) gps_lat_i % 100;
            short lon_D = (int) gps_lon_f;
            float temp_lon_M = (gps_lon_f - lon_D) * 100;
            unsigned char lon_M = (int) temp_lon_M;
            unsigned char lon_S = (int) gps_lon_i % 100;
            unsigned char gps_fix = fix_num;
            char buff[150];

            lora_trans[0] = (app_id << 4) | (temperatureLora >> 6);
            lora_trans[1] = (temperatureLora << 2) | (humiditylora >> 8);
            lora_trans[2] = humiditylora;
            // END FOR THE APP_ID, TEMPERATURE AND HUMIDITY
            lora_trans[3] = pm25lora >> 3;
            lora_trans[4] = (pm25lora << 5) | (pm100Offset >> 3);
            // END FOR PM2.5    
            lora_trans[5] = (pm100Offset << 5) | (lat_D >> 3);
            lora_trans[6] = (lat_D << 5) | (lat_M >> 1);
            lora_trans[7] = (lat_M << 7) | (lat_S << 1) | (lon_D >> 8);
            lora_trans[8] = (unsigned char) lon_D;
            lora_trans[9] = (lon_M << 2) | (lon_S >> 4);
            lora_trans[10] = (lon_S << 4) | gps_fix;
            // END FOR PM10 AND GPS
            sprintf(buff, "AT+DTX=22,%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\r\n", lora_trans[0], lora_trans[1], lora_trans[2], lora_trans[3], lora_trans[4], lora_trans[5], lora_trans[6], lora_trans[7], lora_trans[8], lora_trans[9], lora_trans[10]);
            lora.printf(buff);
            pc.printf(buff);
            count = 0;
            rgbLCD.clear();
            rgbLCD.locate(0,0);
            rgbLCD.print("LoRa Send");
            }
            else if (count == 14){
                count = 0;
                }
        count++;
    }
}
