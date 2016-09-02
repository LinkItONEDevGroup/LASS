##Geiger mbed NXP LPC1768 實驗步驟

* 架構：

	感測器 -> LPC1768 -> Print數值

* POCKET Geiger 官方網站：http://www.radiation-watch.org/2011/05/professional.html

* 硬體資料與接線圖：http://radiation-watch.sakuraweb.com/share/type5_connect_with_arduino.pdf

* 開發平台：https://developer.mbed.org/

* 操作：

	點選Compiler -> 點選Import -> Search criteria -> LASS_Geiger_mbed 
	-> 或在https://developer.mbed.org/users/royedfa5229/code/LASS_Geiger_mbed/ (點選Import)
	-> 可更改Import Name -> main.cpp為主程式 
	-> 按下Compile(注意！右上角為你使用的開發板) 
	-> 將下載下來的檔案放進開發板模擬的USB隨身碟 
	-> 重新插拔及燒入完成

##程式說明與檔案內容物

* main.cpp為主程式
* 此程式主要是將原官網提供的Arduino code 轉換成 mbed 能使用