# LASS7688 Project

This is the main page for the LASS7688 project. The development of this project is based on the AnySense_7688 project. The project is distributed in an open sourced manner, and you are welcome to contribute and help us!

## Change log

* 2018/1/7: version 0.1 released (use G5T, and support OTA, SD storage, OLED, restful data uploading)

## Hardware

* MediaTek LinkIt Smart 7688
* MediaTek LinkIt Smart 7688 Breakout (optional)
* T/H/PM sensor: Plantower PMS5003T
  * https://www.taiwaniot.com.tw/product/pms5003t-g5t-攀藤粉塵溫濕度二合一感測器/
  * http://mntech.com.tw/PMS5003T
* 0.96" OLED (optional)
  * https://www.taiwaniot.com.tw/product/0-96吋oled-液晶屏顯示模組/
  * http://www.icshop.com.tw/product_info.php/products_id/12424
* SD card (optional)

## How to install the codes (for new LinkIt Smart 7688 boards)?

Please login your 7688 board, and switch your 7688 to station mode (note: the other choice is to turn off WiFi and enable Ethernet connection). Then, please change the working directory to /root and follow the following steps:

1. Use the coommand to get the latest version of the codes: 
   ```
   git clone https://github.com/cclljj/LASS7688

2. update the values of GPS_LAT and GPS_LON with your GPS coordinates in the file /root/LASS7688/APP_LASS7688_config.py

3. Run the setup script:
   ```
   /root/LASS7688/setup.sh

Then, the script will install and configure everything. The system will reboot after the installation is done, and hopefully the system will start to upload data to the backend database.

## How to re-install the codes?

Please use the designated account/password to login the system, and remove the existing project source codes:
