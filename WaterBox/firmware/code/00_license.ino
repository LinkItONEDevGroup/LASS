/***<< license >>*****************************************
   Author：Kyle
   Licenses：MIT
   Version：1.0 (2018507)
*********************************************************/
/***<< Revision >>*****************************************
   Update：
   V 1.0
   1.專案開始
   2.外部函式庫
   2.1  U8g2.h        SSD1306 (螢幕顯示)
   2.2  OneWire.h     DS18B20 (溫度)
   2.3  SPI.h SD.h    SD卡
   2.4  RTClib.h      DS3231  (時間模組)
   3. 腳位設定
   3.1  A0：pH meter
   3.2  A1：EC meter
   3.3  A2：Rotary button 1
   3.4  A3：Rotary button 2
   3.5  P2：DS18B20
   3.6  P3：SwitchPin
   3.7  P4：Relay control(Reserved) sensor電源控制
   3.5  P5：(Reserved)
   3.6  其他：SPI, I2C
   4. EEPROM頁面設定
   4.1  P0: pH_slop
   4.2  P1: pH_intercept
   4.3  P2: EC_slop
   4.4  P3: EC_intercept
   4.5  P4: pH_alarm
   4.6  P5: EC_alarm
*********************************************************/
/***<< 版本歷史 >>*****************************************
  V 1.0
  1.專案開始
   2.外部函式庫
   2.1  U8g2.h        SSD1306 (螢幕顯示)
   2.2  OneWire.h     DS18B20 (溫度)
   2.3  SPI.h SD.h    SD卡
   2.4  RTClib.h      DS3231  (時間模組)
   3. 腳位設定
   4. EEPROM頁面設定
   4.1  P0: pH_slop
   4.2  P1: pH_intercept
   4.3  P2: EC_slop
   4.4  P3: EC_intercept
   4.5  P4: pH_alarm 
   4.6  P5: EC_alarm
*********************************************************/
