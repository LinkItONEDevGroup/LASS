/***<<  腳位設定 (MTDuino) >>*****************************************
   保留：2,3,9,  Serial2： 12 (Rx) and 10 (Tx),  Serial3： 5 (Rx) and 4 (Tx)
   5:   sigfox module 供電
   6：  SwitchPin
   7：  DS18B20
   8:   USR_pin
   10： SPI的SS   (SD卡保留)
   11： SPI的MOSI (SD卡保留)
   12： SPI的MISO (SD卡保留)
   13： SPI的SCK  (SD卡保留)
   A0： pH meter
   A1： EC meter
   A2： Rotary button 1
   A3： Rotary button 2
   A4： 待確認能不能用(I2C SDA)
   A5： 待確認能不能用(I2C SCL)
   SCL：I2C(螢幕)
   SDA：I2C(螢幕)
***************************************************************************/
#include <RTClib.h>            // DS3231 library
#include <OneWire.h>          // DS18B20 library
#include <SPI.h>              // for SD Card
#include <SD.h>               // SD Card library

// pin setting
#define pH_pin        A0
#define EC_pin        A1
#define Rotary_Pin_1   A2
#define Rotary_Pin_2   A3

#define RF_PWEN  5            // sigfox power control pin D5
#define switch_pin   6
#define SD_CS       10        // CS pin for SD
#define DS18B20_Pin  7
#define USR_pin      8

int ADC_voltage = 3300;       // ADC的輸入電壓上限
int ADC_range = 4095;         // ADC的輸出範圍
int r_ADC_range = 2500;       // 旋鈕的輸出範圍
int ADC_offset = 30;          // ADC讀值偏移量

// library seting
#include <Arduino.h>                  // required before wiring_private.h
#include "wiring_private.h"            // pinPeripheral() function
#define EEPROM_deviceaddress 0x57       // AT24C32 EEPROM Address in DS3231

// firmware Version
const char* _firwareVersion = "Ver 1.1";

float pH_slop, pH_intercept, EC_slop, EC_intercept, pH_alarm, EC_alarm;

// 頻率設定
int print_interval = 1000;               //  SerialUSB 列印頻率
int SD_save_interval = 300;              // 5分鐘存SD卡(5*60)
unsigned int Upload_interval = 1200;      // 20分鐘上傳sigfox(20*60)

// 資料格式定義 APPID+FiledDefinition+動態欄位
int App_ID = 0x6;                       // App ID 給LASS後端辨識用
int Temperature_filed[2] = {1,8};         // [0]:啟用狀態(1:yes; 0:no), [1]:資料長度(bits)
int pH_filed[2] = {1,8};                 // [0]:啟用狀態(1:yes; 0:no), [1]:資料長度(bits)
int EC_filed[2] = {1,18};                // [0]:啟用狀態(1:yes; 0:no), [1]:資料長度(bits)
int DO_filed[2] = {0,7};                 // [0]:啟用狀態(1:yes; 0:no), [1]:資料長度(bits)
int ORP_filed[2] = {0,11};               // [0]:啟用狀態(1:yes; 0:no), [1]:資料長度(bits)
const int Filed_length_limit = 52;        // Filed length limit (在設定時檢查是否超過長度，16*4(sigfox) - 4(app id)- 8(FiledDefinition) =52)

// DS3231 initial setting
RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

//DS18B20 initial setting, Signal pin on D2
OneWire ds(DS18B20_Pin);

// SSD 1306 OLED library
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_SSD1306.h"

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

