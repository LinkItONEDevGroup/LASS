##LASS mbed NXP LPC1768 實驗步驟

*架構：

		感測器 -> LPC1768 -> 使用LoRa送出(Gemtek GL6509) -> Gemtek MQTT Server -> Topic Gemtek MQTT -> 抓取回來解析後重新包裝成LASS格式 -> 上傳至LASS MQTT Server

*硬體資料與接線圖：https://hackpad.com/LASS-mbed-NXP-LPC1768-mDoiODkeBzF

*開發平台：https://developer.mbed.org/

*操作：


點選Compiler -> 點選Import -> 點選Click here -> 在Source URL貼上
http://developer.mbed.org/users/royedfa5229/code/LASS_RoLa_mbed/ (也可以使用Upload將檔案上傳) -> 可更改Import Name -> main.cpp為主程式 -> 按下Compile(注意！右上角為你使用的開發板) -> 將下載下來的檔案放進開發板模擬的USB隨身碟 -> 重新插拔及燒入完成

*觀看LCD即可知道數值,若需要更詳細觀察可下載【CoolTermWin】來使用

##程式說明與檔案內容物
*main.cpp為主程式

*請更改char gps_lat[] = "自己的緯度";char gps_lon[] = "自己的經度";

*若是使用不同mbed或是想更換Pin腳，可更改pin為自己要的pin腳Serial device(pin, pin);Serial lora(pin, pin);DHT dht22(pin, SEN51035P);Grove_LCD_RGB_Backlight rgbLCD(pin, pin);

*LASSwithLoRa底下的DHT、Grove_LCD_RGB_Backlight、mbed為Libraries

##測試

*請看測試部分：https://hackpad.com/LASS-mbed-NXP-LPC1768-mDoiODkeBzF