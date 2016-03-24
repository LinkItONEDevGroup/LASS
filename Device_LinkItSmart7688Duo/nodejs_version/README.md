LASS Linkit 7688 Duo 實驗步驟

* 請參考文件： https://hackpad.com/1HcedS5HQnI

* 原理：MCU負責感測並送至MPU=>MPU之NodeJS負責對時，組MQTT上傳
* 準備：7688 Duo/G3/DHT/行動電源
* 連接六條硬體接線
* 7688Duo=>設定為Station Mode並設定連上手機分享以上網
* Copy DHT目錄至Arduino Library
* 開啟LASS-7688Duo.ino並上傳
* 編輯iot.js 改 deviceId: "TEST_001"=>改成你的device id
* scp iot.js至7688 root
* ssh至7688
* (以下看能否改為install.sh)
opkg update
opkg install mosquitto mosquitto-client libmosquitto
npm install mqtt --save
npm install ntp-client --save

##編輯/etc/rc.local
#!/bin/sh -e
node /root/iot.js
exit 0
##

chown 777 /etc/rc.local
reboot -f

* 用mqtt-spy工具連上gpssensor.ddns.net看 
  LASS/Test/PM25 或 LASS/Test/PM25/TEST_001 (兩邊都會有資訊)

* Todo:
* 利用手機更新GPS
* 手機介面
